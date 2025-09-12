import numpy as np
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import tkinter as tk
from tkinter import filedialog, messagebox
import re
import cmath

class SignalAnalyzerApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Визуализатор сигналов и корреляции")
        self.root.geometry("1200x900")

        # Обработка закрытия окна
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)

        self.signal1_data = None
        self.signal2_data = None
        self.correlation_data = None

        self.create_widgets()

    def create_widgets(self):
        # Фрейм для кнопок
        button_frame = tk.Frame(self.root)
        button_frame.pack(pady=10)

        # Кнопки для загрузки файлов
        self.load_btn1 = tk.Button(button_frame, text="Загрузить сигнал 1 (I/Q)", 
                                  command=lambda: self.load_signal_file(1))
        self.load_btn1.pack(side=tk.LEFT, padx=5)

        self.load_btn2 = tk.Button(button_frame, text="Загрузить сигнал 2 (I/Q)", 
                                  command=lambda: self.load_signal_file(2))
        self.load_btn2.pack(side=tk.LEFT, padx=5)

        self.load_corr_btn = tk.Button(button_frame, text="Загрузить корреляцию", 
                                      command=self.load_correlation_file)
        self.load_corr_btn.pack(side=tk.LEFT, padx=5)

        # Кнопка отображения
        self.show_btn = tk.Button(button_frame, text="Показать графики", 
                                 command=self.show_plots,
                                 state=tk.DISABLED)
        self.show_btn.pack(side=tk.LEFT, padx=5)

        # Кнопка выхода
        self.exit_btn = tk.Button(button_frame, text="Выход", 
                                 command=self.on_closing,
                                 bg='red', fg='white')
        self.exit_btn.pack(side=tk.LEFT, padx=5)

        # Создаем фреймы для трех отдельных графиков
        self.plot_frame = tk.Frame(self.root)
        self.plot_frame.pack(fill=tk.BOTH, expand=True)

        # Фреймы для каждого графика
        self.frame1 = tk.Frame(self.plot_frame)
        self.frame1.pack(side=tk.TOP, fill=tk.BOTH, expand=True, padx=5, pady=5)

        self.frame2 = tk.Frame(self.plot_frame)
        self.frame2.pack(side=tk.TOP, fill=tk.BOTH, expand=True, padx=5, pady=5)

        self.frame3 = tk.Frame(self.plot_frame)
        self.frame3.pack(side=tk.TOP, fill=tk.BOTH, expand=True, padx=5, pady=5)

        # Создаем отдельные фигуры для каждого графика
        self.fig1, self.ax1 = plt.subplots(figsize=(12, 4))
        self.fig2, self.ax2 = plt.subplots(figsize=(12, 4))
        self.fig3, self.ax3 = plt.subplots(figsize=(12, 4))

        # Canvas для каждого графика
        self.canvas1 = FigureCanvasTkAgg(self.fig1, master=self.frame1)
        self.canvas1.get_tk_widget().pack(fill=tk.BOTH, expand=True)

        self.canvas2 = FigureCanvasTkAgg(self.fig2, master=self.frame2)
        self.canvas2.get_tk_widget().pack(fill=tk.BOTH, expand=True)

        self.canvas3 = FigureCanvasTkAgg(self.fig3, master=self.frame3)
        self.canvas3.get_tk_widget().pack(fill=tk.BOTH, expand=True)

    def on_closing(self):
        """Обработчик закрытия приложения"""
        if messagebox.askokcancel("Выход", "Вы уверены, что хотите выйти?"):
            self.root.quit()
            self.root.destroy()

    def parse_complex_txt_file(self, filename):
        """Парсинг файла с комплексными числами в формате (real,imag)"""
        complex_data = []
        try:
            with open(filename, 'r', encoding='utf-8') as file:
                content = file.read()

                # Ищем все вхождения в формате (число,число)
                pattern = r'\(([-\d\.]+),([-\d\.]+)\)'
                matches = re.findall(pattern, content)

                for real_str, imag_str in matches:
                    try:
                        real = float(real_str.strip())
                        imag = float(imag_str.strip())
                        complex_data.append(complex(real, imag))
                    except ValueError:
                        continue

                if len(complex_data) == 0:
                    # Альтернативный вариант: попробовать парсить построчно
                    file.seek(0)
                    for line in file:
                        line = line.strip()
                        if not line or line.startswith('#'):
                            continue

                        # Пытаемся найти числа в любом формате
                        numbers = re.findall(r'[-+]?\d*\.?\d+(?:[eE][-+]?\d+)?', line)
                        if len(numbers) >= 2:
                            try:
                                real = float(numbers[0])
                                imag = float(numbers[1])
                                complex_data.append(complex(real, imag))
                            except ValueError:
                                continue

            return np.array(complex_data)

        except Exception as e:
            messagebox.showerror("Ошибка", f"Ошибка при чтении файла {filename}:\n{str(e)}")
            return None

    def parse_real_txt_file(self, filename):
        """Парсинг файла с действительными числами в формате num, num, num"""
        real_data = []
        try:
            with open(filename, 'r', encoding='utf-8') as file:
                content = file.read()

                # Ищем все числа в тексте
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

    def load_signal_file(self, signal_num):
        """Загрузка файла с комплексными данными сигнала"""
        filename = filedialog.askopenfilename(
            title=f"Выберите файл для сигнала {signal_num} (формат: (real,imag))",
            filetypes=[("Текстовые файлы", "*.txt"), ("Все файлы", "*.*")]
        )
        if filename:
            data = self.parse_complex_txt_file(filename)
            if data is not None and len(data) > 0:
                if signal_num == 1:
                    self.signal1_data = data
                    messagebox.showinfo("Успех", 
                        f"Сигнал 1 загружен.\nТочек: {len(data)}\n"
                        f"Диапазон I: [{np.real(data).min():.3f}, {np.real(data).max():.3f}]\n"
                        f"Диапазон Q: [{np.imag(data).min():.3f}, {np.imag(data).max():.3f}]")
                else:
                    self.signal2_data = data
                    messagebox.showinfo("Успех", 
                        f"Сигнал 2 загружен.\nТочек: {len(data)}\n"
                        f"Диапазон I: [{np.real(data).min():.3f}, {np.real(data).max():.3f}]\n"
                        f"Диапазон Q: [{np.imag(data).min():.3f}, {np.imag(data).max():.3f}]")

                self.check_data_loaded()

    def load_correlation_file(self):
        """Загрузка файла с данными корреляции"""
        filename = filedialog.askopenfilename(
            title="Выберите файл с данными корреляции (формат: num, num, num)",
            filetypes=[("Текстовые файлы", "*.txt"), ("Все файлы", "*.*")]
        )

        if filename:
            data = self.parse_real_txt_file(filename)
            if data is not None and len(data) > 0:
                self.correlation_data = data
                messagebox.showinfo("Успех", 
                    f"Данные корреляции загружены.\nТочек: {len(data)}\n"
                    f"Диапазон: [{data.min():.3f}, {data.max():.3f}]\n"
                    f"Максимум: {data.max():.3f} на отсчете {data.argmax()}")

                self.check_data_loaded()

    def check_data_loaded(self):
        """Проверка, все ли данные загружены"""
        if (self.signal1_data is not None and 
            self.signal2_data is not None and 
            self.correlation_data is not None):
            self.show_btn.config(state=tk.NORMAL)

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
            self.ax1.set_title('Сигнал 1 - Комплексные I/Q компоненты')
            self.ax1.set_xlabel('Отсчеты')
            self.ax1.set_ylabel('Амплитуда')
            self.ax1.legend()
            self.ax1.grid(True, alpha=0.3)

            # 2. График второго комплексного сигнала
            time2 = np.arange(len(self.signal2_data))
            self.ax2.plot(time2, np.real(self.signal2_data), 'g-', label='Сигнал 2 (I)', linewidth=1)
            self.ax2.plot(time2, np.imag(self.signal2_data), 'm-', label='Сигнал 2 (Q)', linewidth=1)
            self.ax2.set_title('Сигнал 2 - Комплексные I/Q компоненты')
            self.ax2.set_xlabel('Отсчеты')
            self.ax2.set_ylabel('Амплитуда')
            self.ax2.legend()
            self.ax2.grid(True, alpha=0.3)

            # 3. График модуля корреляции
            time_corr = np.arange(len(self.correlation_data))
            self.ax3.plot(time_corr, self.correlation_data, 'orange', linewidth=2)
            self.ax3.set_title('Модуль корреляции сигналов')
            self.ax3.set_xlabel('Отсчеты')
            self.ax3.set_ylabel('Амплитуда корреляции')
            self.ax3.grid(True, alpha=0.3)

            # Помечаем максимум корреляции
            max_idx = np.argmax(self.correlation_data)
            max_val = self.correlation_data[max_idx]
            self.ax3.plot(max_idx, max_val, 'ro', markersize=8)
            self.ax3.annotate(f'Максимум: {max_val:.3f}\nОтсчет: {max_idx}', 
                             xy=(max_idx, max_val),
                             xytext=(max_idx + len(self.correlation_data)*0.1, max_val*0.9),
                             arrowprops=dict(arrowstyle='->', color='red'),
                             bbox=dict(boxstyle="round,pad=0.3", fc="yellow", alpha=0.3))

            # Обновляем canvas
            self.canvas1.draw()
            self.canvas2.draw()
            self.canvas3.draw()

        except Exception as e:
            messagebox.showerror("Ошибка", f"Ошибка при отображении графиков:\n{str(e)}")

def main():
    root = tk.Tk()
    app = SignalAnalyzerApp(root)
    root.mainloop()

if __name__ == "__main__":
    main()