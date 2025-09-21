import numpy as np
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import tkinter as tk
from tkinter import filedialog, messagebox
from tkinter import ttk
import re
import cmath
import subprocess
import os
import threading
import time

class SignalAnalyzerApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Обработчик сигналов с внешней утилитой")
        
        # Настройки путей
        self.build_dir = "build"
        self.data_dir = "data"
        self.processing_app = "./data_processing"
        
        # Настройки размеров
        self.main_window_size = "1600x1000"
        self.figure_sizes = [(14, 2), (14, 2), (14, 2)]
        self.frame_padding = {"padx": 13, "pady": 8}
        
        # Параметры по умолчанию
        self.params = {
            "fd": 20.0,    # Sample freq
            "f": 10.0,     # Carrier freq
            "n": 10,       # Num info bits
            "vel": 10.,   # Info velocity
            "dt": 0.0,    # Time offset
            "snr1": 10,  # SNR for signal 1
            "snr2": 10.0,   # SNR for signal 2
            "type": 0     # modulation type 
        }
        
        self.root.geometry(self.main_window_size)
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)
        
        self.signal1_data = None
        self.signal2_data = None
        self.correlation_data = None
        
        self.create_widgets()
        
    def create_widgets(self):
        # Фрейм для параметров
        params_frame = tk.LabelFrame(self.root, text="Параметры обработки", font=('Arial', 10))
        params_frame.pack(fill=tk.X, padx=10, pady=5)
        
        # Создаем поля для параметров
        param_labels = {
            "fd": "Частота дискретизации (fd):",
            "f": "Несущая частота (f):",
            "n": "Количество бит информации (n):",
            "vel": "Скорость информации (vel):",
            "dt": "Временное смещение (dt):",
            "snr1": "SNR для сигнала 1 (snr1):",
            "snr2": "SNR для сигнала 2 (snr2):",
            "type": "Тип модуляции (0 - АМ, 1 - ФМ-2, 2 - МЧМ):"
        }
        
        self.param_entries = {}
        row = 0
        col = 0
        
        for i, (key, label) in enumerate(param_labels.items()):
            frame = tk.Frame(params_frame)
            frame.grid(row=row, column=col, padx=5, pady=2, sticky="w")
            
            tk.Label(frame, text=label, font=('Arial', 9)).pack(side=tk.LEFT)
            entry = tk.Entry(frame, width=10, font=('Arial', 9))
            entry.insert(0, str(self.params[key]))
            entry.pack(side=tk.LEFT, padx=5)
            self.param_entries[key] = entry
            
            col += 1
            if col > 2:  # 3 колонки в ряду
                col = 0
                row += 1
        
        # Фрейм для кнопок
        button_frame = tk.Frame(self.root)
        button_frame.pack(pady=10)
        
        # Кнопка старт
        self.start_btn = tk.Button(button_frame, text="Старт обработки", 
                                  command=self.start_processing,
                                  bg='green', fg='white', font=('Arial', 12))
        self.start_btn.pack(side=tk.LEFT, padx=10)
        
        # Кнопка выхода
        self.exit_btn = tk.Button(button_frame, text="Выход", 
                                 command=self.on_closing,
                                 bg='red', fg='white', font=('Arial', 12))
        self.exit_btn.pack(side=tk.LEFT, padx=10)
        
        # Статус бар
        self.status_var = tk.StringVar()
        self.status_var.set("Готов к работе")
        self.status_bar = tk.Label(self.root, textvariable=self.status_var, 
                                  relief=tk.SUNKEN, anchor=tk.W, font=('Arial', 10))
        self.status_bar.pack(side=tk.BOTTOM, fill=tk.X)
        
        # Прогресс бар
        self.progress_var = tk.DoubleVar()
        self.progress_bar = ttk.Progressbar(self.root, variable=self.progress_var, 
                                          maximum=100, mode='determinate')
        self.progress_bar.pack(side=tk.BOTTOM, fill=tk.X, padx=10, pady=5)
        
        # Создаем фреймы для трех отдельных графиков
        self.plot_frame = tk.Frame(self.root)
        self.plot_frame.pack(fill=tk.BOTH, expand=True)
        
        # Фреймы для каждого графика
        self.frame1 = tk.Frame(self.plot_frame)
        self.frame1.pack(side=tk.TOP, fill=tk.BOTH, expand=True, **self.frame_padding)
        
        self.frame2 = tk.Frame(self.plot_frame)
        self.frame2.pack(side=tk.TOP, fill=tk.BOTH, expand=True, **self.frame_padding)
        
        self.frame3 = tk.Frame(self.plot_frame)
        self.frame3.pack(side=tk.TOP, fill=tk.BOTH, expand=True, **self.frame_padding)
        
        # Создаем отдельные фигуры
        self.fig1, self.ax1 = plt.subplots(figsize=self.figure_sizes[0])
        self.fig2, self.ax2 = plt.subplots(figsize=self.figure_sizes[1])
        self.fig3, self.ax3 = plt.subplots(figsize=self.figure_sizes[2])
        
        # Настраиваем отступы внутри фигур
        figure_padding = 1.0
        self.fig1.tight_layout(pad=figure_padding)
        self.fig2.tight_layout(pad=figure_padding)
        self.fig3.tight_layout(pad=figure_padding)
        
        # Canvas для каждого графика
        self.canvas1 = FigureCanvasTkAgg(self.fig1, master=self.frame1)
        self.canvas1.get_tk_widget().pack(fill=tk.BOTH, expand=True)
        
        self.canvas2 = FigureCanvasTkAgg(self.fig2, master=self.frame2)
        self.canvas2.get_tk_widget().pack(fill=tk.BOTH, expand=True)
        
        self.canvas3 = FigureCanvasTkAgg(self.fig3, master=self.frame3)
        self.canvas3.get_tk_widget().pack(fill=tk.BOTH, expand=True)
    
    def get_parameters(self):
        """Получение параметров из полей ввода"""
        try:
            params = {}
            for key, entry in self.param_entries.items():
                value = entry.get().strip()
                if key == "n":
                    params[key] = int(value) if value else 0
                else:
                    params[key] = float(value) if value else 0.0
            return params
        except ValueError as e:
            messagebox.showerror("Ошибка", f"Некорректное значение параметра: {str(e)}")
            return None
    
    def on_closing(self):
        """Обработчик закрытия приложения"""
        if messagebox.askokcancel("Выход", "Вы уверены, что хотите выйти?"):
            self.root.quit()
            self.root.destroy()
    
    def check_directories(self):
        """Проверка существования необходимых директорий"""
        if not os.path.exists(self.build_dir):
            messagebox.showerror("Ошибка", f"Директория {self.build_dir} не существует!")
            return False
        
        if not os.path.exists(os.path.join(self.build_dir, self.processing_app)):
            messagebox.showerror("Ошибка", 
                               f"Утилита {self.processing_app} не найдена в {self.build_dir}!")
            return False
        
        # Создаем data директорию если ее нет
        if not os.path.exists(self.data_dir):
            os.makedirs(self.data_dir)
            self.status_var.set(f"Создана директория {self.data_dir}")
        
        return True
    
    def run_data_processing(self, params):
        """Запуск внешней утилиты обработки данных с параметрами"""
        try:
            # Переходим в build директорию
            original_dir = os.getcwd()
            os.chdir(self.build_dir)
            
            # Формируем аргументы командной строки
            args = [
                self.processing_app,
                str(params["fd"]),
                str(params["f"]),
                str(params["n"]),
                str(params["vel"]),
                str(params["dt"]),
                str(params["snr1"]),
                str(params["snr2"]),
                str(params["type"])
            ]
            
            # Запускаем утилиту
            self.status_var.set("Запуск data_processing...")
            self.progress_var.set(20)
            
            result = subprocess.run(args, 
                                  capture_output=True, 
                                  text=True, 
                                  timeout=30)  # Таймаут 30 секунд
            
            # Возвращаемся обратно
            os.chdir(original_dir)
            
            if result.returncode != 0:
                error_msg = f"Ошибка выполнения {self.processing_app}:\n{result.stderr}"
                messagebox.showerror("Ошибка", error_msg)
                return False
            
            self.status_var.set("Утилита выполнена успешно")
            self.progress_var.set(40)
            return True
            
        except subprocess.TimeoutExpired:
            os.chdir(original_dir)
            messagebox.showerror("Ошибка", "Утилита превысила время выполнения!")
            return False
        except Exception as e:
            os.chdir(original_dir)
            messagebox.showerror("Ошибка", f"Ошибка при запуске утилиты: {str(e)}")
            return False
    
    def parse_complex_txt_file(self, filename):
        """Парсинг файла с комплексными числами в формате (real,imag)"""
        complex_data = []
        try:
            with open(filename, 'r', encoding='utf-8') as file:
                content = file.read()
                pattern = r'\(([-\d\.]+),([-\d\.]+)\)'
                matches = re.findall(pattern, content)
                
                for real_str, imag_str in matches:
                    try:
                        real = float(real_str.strip())
                        imag = float(imag_str.strip())
                        complex_data.append(complex(real, imag))
                    except ValueError:
                        continue
            
            return np.array(complex_data)
            
        except Exception as e:
            messagebox.showerror("Ошибка", f"Ошибка при чтении файла {filename}:\n{str(e)}")
            return None
    
    def parse_real_txt_file(self, filename):
        """Парсинг файла с действительными числами"""
        real_data = []
        try:
            with open(filename, 'r', encoding='utf-8') as file:
                content = file.read()
                numbers = re.findall(r'[-+]?\d*\.?\d+(?:[eE][-+]?\d+)?', content)
                
                for num_str in numbers:
                    try:
                        real_data.append(float(num_str))
                    except ValueError:
                        continue
            
            return np.array(real_data)
            
        except Exception as e:
            messagebox.showerror("Ошибка", f"Ошибка при чтении файла {filename}:\n{str(e)}")
            return None
    
    def load_generated_files(self):
        """Загрузка сгенерированных файлов"""
        files_to_load = [
            ("first_data.txt", "signal1"),
            ("second_data.txt", "signal2"), 
            ("correlation.txt", "correlation")
        ]
        
        for filename, data_type in files_to_load:
            filepath = os.path.join(self.data_dir, filename)
            
            if not os.path.exists(filepath):
                messagebox.showerror("Ошибка", f"Файл {filename} не найден в {self.data_dir}!")
                return False
            
            self.status_var.set(f"Загрузка {filename}...")
            
            if data_type in ["signal1", "signal2"]:
                data = self.parse_complex_txt_file(filepath)
            else:
                data = self.parse_real_txt_file(filepath)
            
            if data is None or len(data) == 0:
                messagebox.showerror("Ошибка", f"Не удалось загрузить данные из {filename}!")
                return False
            
            if data_type == "signal1":
                self.signal1_data = data
            elif data_type == "signal2":
                self.signal2_data = data
            else:
                self.correlation_data = data
            
            self.progress_var.set(self.progress_var.get() + 15)
            time.sleep(0.1)  # Небольшая задержка для визуализации прогресса
        
        return True
    
    def show_plots(self):
        """Отображение графиков"""
        try:
            # Очищаем графики
            self.ax1.clear()
            self.ax2.clear()
            self.ax3.clear()
            
            # 1. График первого комплексного сигнала
            time1 = np.arange(len(self.signal1_data))
            self.ax1.plot(time1, np.real(self.signal1_data), 'b-', label='Сигнал 1 (I)', linewidth=1)
            self.ax1.plot(time1, np.imag(self.signal1_data), 'r-', label='Сигнал 1 (Q)', linewidth=1)
            self.ax1.set_title('Сигнал 1 - Комплексные I/Q компоненты', fontsize=12)
            self.ax1.set_xlabel('Отсчеты', fontsize=10)
            self.ax1.set_ylabel('Амплитуда', fontsize=10)
            self.ax1.legend(fontsize=9)
            self.ax1.grid(True, alpha=0.3)
            
            # 2. График второго комплексного сигнала
            time2 = np.arange(len(self.signal2_data))
            self.ax2.plot(time2, np.real(self.signal2_data), 'g-', label='Сигнал 2 (I)', linewidth=1)
            self.ax2.plot(time2, np.imag(self.signal2_data), 'm-', label='Сигнал 2 (Q)', linewidth=1)
            self.ax2.set_title('Сигнал 2 - Комплексные I/Q компоненты', fontsize=12)
            self.ax2.set_xlabel('Отсчеты', fontsize=10)
            self.ax2.set_ylabel('Амплитуда', fontsize=10)
            self.ax2.legend(fontsize=9)
            self.ax2.grid(True, alpha=0.3)
            
            # 3. График модуля корреляции
            time_corr = np.arange(len(self.correlation_data))
            self.ax3.plot(time_corr, self.correlation_data, 'orange', linewidth=2)
            self.ax3.set_title('Модуль корреляции сигналов', fontsize=12)
            self.ax3.set_xlabel('Отсчеты', fontsize=10)
            self.ax3.set_ylabel('Амплитуда корреляции', fontsize=10)
            self.ax3.grid(True, alpha=0.3)
            
            # Помечаем максимум корреляции
            max_idx = np.argmax(self.correlation_data)
            max_val = self.correlation_data[max_idx]
            self.ax3.plot(max_idx, max_val, 'ro', markersize=8)
            self.ax3.annotate(f'Максимум: {max_val:.3f}\nОтсчет: {max_idx}', 
                             xy=(max_idx, max_val),
                             xytext=(max_idx + len(self.correlation_data)*0.1, max_val*0.9),
                             arrowprops=dict(arrowstyle='->', color='red'),
                             bbox=dict(boxstyle="round,pad=0.3", fc="yellow", alpha=0.3),
                             fontsize=9)
            
            # Обновляем canvas
            self.fig1.tight_layout()
            self.fig2.tight_layout()
            self.fig3.tight_layout()
            
            self.canvas1.draw()
            self.canvas2.draw()
            self.canvas3.draw()
            
            self.status_var.set("Графики успешно отображены")
            self.progress_var.set(100)
            
        except Exception as e:
            messagebox.showerror("Ошибка", f"Ошибка при отображении графиков:\n{str(e)}")
    
    def processing_thread(self):
        """Поток обработки данных"""
        try:
            self.start_btn.config(state=tk.DISABLED)
            self.progress_var.set(0)
            
            # Получаем параметры
            self.status_var.set("Чтение параметров...")
            params = self.get_parameters()
            if params is None:
                return
            
            # Шаг 1: Проверка директорий
            self.status_var.set("Проверка директорий...")
            if not self.check_directories():
                return
            
            self.progress_var.set(10)
            
            # Шаг 2: Запуск внешней утилиты с параметрами
            self.status_var.set("Запуск data_processing с параметрами...")
            if not self.run_data_processing(params):
                return
            
            # Шаг 3: Загрузка сгенерированных файлов
            self.status_var.set("Загрузка данных...")
            if not self.load_generated_files():
                return
            
            # Шаг 4: Отображение графиков
            self.status_var.set("Построение графиков...")
            self.show_plots()
            
            messagebox.showinfo("Успех", "Обработка данных завершена успешно!")
            
        except Exception as e:
            messagebox.showerror("Ошибка", f"Ошибка в процессе обработки: {str(e)}")
        finally:
            self.start_btn.config(state=tk.NORMAL)
    
    def start_processing(self):
        """Запуск процесса обработки в отдельном потоке"""
        thread = threading.Thread(target=self.processing_thread)
        thread.daemon = True
        thread.start()

def main():
    root = tk.Tk()
    app = SignalAnalyzerApp(root)
    root.mainloop()

if __name__ == "__main__":
    main()