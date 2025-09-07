import numpy as np
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import tkinter as tk
from tkinter import filedialog, messagebox
import re
import cmath
import sys
import os

class SignalAnalyzerApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Анализатор комплексных сигналов")
        self.root.geometry("1400x900")
        
        # Обработка закрытия окна
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)
        
        self.signal1_data = None
        self.signal2_data = None
        
        self.create_widgets()
        
    def create_widgets(self):
        # Фрейм для кнопок
        button_frame = tk.Frame(self.root)
        button_frame.pack(pady=10)
        
        # Кнопки для загрузки файлов
        self.load_btn1 = tk.Button(button_frame, text="Загрузить сигнал 1 (I/Q)", 
                                  command=lambda: self.load_file(1))
        self.load_btn1.pack(side=tk.LEFT, padx=5)
        
        self.load_btn2 = tk.Button(button_frame, text="Загрузить сигнал 2 (I/Q)", 
                                  command=lambda: self.load_file(2))
        self.load_btn2.pack(side=tk.LEFT, padx=5)
        
        # Кнопка старт
        self.start_btn = tk.Button(button_frame, text="Старт анализ", 
                                  command=self.start_analysis,
                                  state=tk.DISABLED)
        self.start_btn.pack(side=tk.LEFT, padx=5)
        
        # Кнопка выхода
        self.exit_btn = tk.Button(button_frame, text="Выход", 
                                 command=self.on_closing,
                                 bg='red', fg='white')
        self.exit_btn.pack(side=tk.LEFT, padx=5)
        
        # Фрейм для графиков
        self.fig = plt.figure(figsize=(12, 10))
        self.grid = plt.GridSpec(3, 2, hspace=0.5, wspace=0.3)
        
        # Создаем subplots
        self.ax1 = self.fig.add_subplot(self.grid[0, :])  # Исходные сигналы
        self.ax2 = self.fig.add_subplot(self.grid[1, 0])  # Амплитуда свертки
        self.ax3 = self.fig.add_subplot(self.grid[1, 1])  # Фаза свертки
        self.ax4 = self.fig.add_subplot(self.grid[2, 0])  # Амплитуда корреляции
        self.ax5 = self.fig.add_subplot(self.grid[2, 1])  # Фаза корреляции
        
        self.canvas = FigureCanvasTkAgg(self.fig, master=self.root)
        self.canvas.draw()
        self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)
    
    def on_closing(self):
        """Обработчик закрытия приложения"""
        if messagebox.askokcancel("Выход", "Вы уверены, что хотите выйти?"):
            self.root.quit()  # Останавливает mainloop
            self.root.destroy()  # Уничтожает окно
            # Принудительное завершение для WSL
            try:
                sys.exit(0)
            except:
                os._exit(0)
        
    def parse_complex_txt_file(self, filename):
        """Парсинг файла с комплексными числами в формате I+Qj или раздельные I и Q"""
        complex_data = []
        try:
            with open(filename, 'r', encoding='utf-8') as file:
                for line_num, line in enumerate(file, 1):
                    line = line.strip()
                    if not line or line.startswith('#'):
                        continue
                    
                    # Пытаемся распарсить разные форматы
                    if 'j' in line or 'i' in line.lower():
                        # Формат: 1.0+2.0j или 1.0-2.0j
                        try:
                            # Заменяем i на j для единообразия
                            line = line.replace('i', 'j').replace('I', 'j')
                            # Убедимся, что есть знак между действительной и мнимой частями
                            if '+' in line and 'j' in line:
                                parts = line.split('+')
                                real = float(parts[0])
                                imag = float(parts[1].replace('j', ''))
                                complex_data.append(complex(real, imag))
                            elif '-' in line[1:] and 'j' in line:
                                # Обрабатываем отрицательную мнимую часть
                                if line.count('-') == 2:  # -1.0-2.0j
                                    parts = line[1:].split('-', 1)
                                    real = -float(parts[0])
                                    imag = -float(parts[1].replace('j', ''))
                                    complex_data.append(complex(real, imag))
                                else:  # 1.0-2.0j
                                    parts = line.split('-', 1)
                                    real = float(parts[0])
                                    imag = -float(parts[1].replace('j', ''))
                                    complex_data.append(complex(real, imag))
                            else:
                                # Просто комплексное число
                                complex_data.append(complex(line))
                        except ValueError:
                            continue
                    else:
                        # Формат: два числа в строке (I и Q)
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
    
    def load_file(self, signal_num):
        """Загрузка файла с комплексными данными"""
        filename = filedialog.askopenfilename(
            title=f"Выберите файл для сигнала {signal_num} (I/Q данные)",
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
                
                if self.signal1_data is not None and self.signal2_data is not None:
                    self.start_btn.config(state=tk.NORMAL)
    
    def complex_convolve(self, a, b):
        """Свертка комплексных сигналов"""
        n = len(a) + len(b) - 1
        result = np.zeros(n, dtype=complex)
        
        for i in range(len(a)):
            for j in range(len(b)):
                result[i + j] += a[i] * b[j]
                
        # Режим 'same' - обрезаем до длины первого сигнала
        start = (len(result) - len(a)) // 2
        return result[start:start + len(a)]
    
    def complex_correlate(self, a, b):
        """Корреляция комплексных сигналов"""
        # Комплексно-сопряженная корреляция
        return self.complex_convolve(a, np.conj(b[::-1]))
    
    def start_analysis(self):
        """Запуск анализа комплексных сигналов"""
        if self.signal1_data is None or self.signal2_data is None:
            messagebox.showerror("Ошибка", "Сначала загрузите оба сигнала!")
            return
        
        try:
            # Очищаем графики
            for ax in [self.ax1, self.ax2, self.ax3, self.ax4, self.ax5]:
                ax.clear()
            
            # 1. Исходные комплексные сигналы
            time = np.arange(len(self.signal1_data))
            self.ax1.plot(time, np.real(self.signal1_data), 'b-', label='Сигнал 1 (I)', linewidth=1)
            self.ax1.plot(time, np.imag(self.signal1_data), 'b--', label='Сигнал 1 (Q)', linewidth=1, alpha=0.7)
            
            time2 = np.arange(len(self.signal2_data))
            self.ax1.plot(time2, np.real(self.signal2_data), 'r-', label='Сигнал 2 (I)', linewidth=1)
            self.ax1.plot(time2, np.imag(self.signal2_data), 'r--', label='Сигнал 2 (Q)', linewidth=1, alpha=0.7)
            
            self.ax1.set_title('Исходные комплексные сигналы (I и Q компоненты)')
            self.ax1.set_xlabel('Отсчеты')
            self.ax1.set_ylabel('Амплитуда')
            self.ax1.legend()
            self.ax1.grid(True, alpha=0.3)
            
            # 2. Свертка комплексных сигналов
            convolution = self.complex_convolve(self.signal1_data, self.signal2_data)
            
            # Амплитуда свертки
            self.ax2.plot(np.abs(convolution), 'g-', linewidth=1)
            self.ax2.set_title('Амплитуда свертки |conv(I,Q)|')
            self.ax2.set_xlabel('Отсчеты')
            self.ax2.set_ylabel('Амплитуда')
            self.ax2.grid(True, alpha=0.3)
            
            # Фаза свертки
            self.ax3.plot(np.angle(convolution), 'm-', linewidth=1)
            self.ax3.set_title('Фаза свертки arg(conv(I,Q))')
            self.ax3.set_xlabel('Отсчеты')
            self.ax3.set_ylabel('Фаза (рад)')
            self.ax3.grid(True, alpha=0.3)
            
            # 3. Корреляция комплексных сигналов
            correlation = self.complex_correlate(self.signal1_data, self.signal2_data)
            
            # Амплитуда корреляции
            self.ax4.plot(np.abs(correlation), 'c-', linewidth=1)
            self.ax4.set_title('Амплитуда корреляции |corr(I,Q)|')
            self.ax4.set_xlabel('Отсчеты')
            self.ax4.set_ylabel('Амплитуда')
            self.ax4.grid(True, alpha=0.3)
            
            # Фаза корреляции
            self.ax5.plot(np.angle(correlation), 'orange', linewidth=1)
            self.ax5.set_title('Фаза корреляции arg(corr(I,Q))')
            self.ax5.set_xlabel('Отсчеты')
            self.ax5.set_ylabel('Фаза (рад)')
            self.ax5.grid(True, alpha=0.3)
            
            # Обновляем canvas
            self.fig.tight_layout()
            self.canvas.draw()
            
            # Показываем информацию о результатах
            max_conv = np.max(np.abs(convolution))
            max_corr = np.max(np.abs(correlation))
            messagebox.showinfo("Результаты анализа", 
                f"Максимум свертки: {max_conv:.3f}\n"
                f"Максимум корреляции: {max_corr:.3f}\n"
                f"Пик корреляции на отсчете: {np.argmax(np.abs(correlation))}")
            
        except Exception as e:
            messagebox.showerror("Ошибка", f"Ошибка при анализе комплексных сигналов:\n{str(e)}")

def main():
    root = tk.Tk()
    app = SignalAnalyzerApp(root)
    root.mainloop()

if __name__ == "__main__":
    main()