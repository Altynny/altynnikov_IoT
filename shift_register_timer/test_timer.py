import tkinter as tk
import serial
import time

class Timer:
    def __init__(self, root: tk.Tk) -> None:
        self.root = root
        self.root.title('Проверка таймера')
        self.root.geometry('300x150')
        
        self.seconds = 0
        
        self.label = tk.Label(
            root, 
            text='0', 
            font=('Arial', 36),
            fg='green'
        )
        self.label.pack(pady=10)
        
        self.start_button = tk.Button(
            root,
            text='Старт',
            command=self.start_timer,
            font=('Arial', 14),
            width=10,
            bg='lightblue'
        )
        self.start_button.pack(pady=10)
        
        self.connection = serial.Serial('COM5', 9600, timeout=1)
        time.sleep(2.5)
    
    def start_timer(self) -> None:
        self.seconds = 0
        self.label.config(text='0', fg='green')
            
        self.connection.write(b'0')
        time.sleep(2.5)
            
        self.update_timer()
    
    def update_timer(self) -> None:
        self.seconds += 1
        self.label.config(text=self.seconds)
        
        self.root.after(1000, self.update_timer)

if __name__ == '__main__':
    root = tk.Tk()
    timer = Timer(root)
    root.mainloop()
