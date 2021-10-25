import requests
from tkinter import *

def enviaDados():
    print('Enviando dados...')

    texto = 'Enviando dados...'

    status_text['text'] = texto

janelaPrincipal = Tk()
janelaPrincipal.title('Sistema de manutenção preditiva - PIBIC/PIBITI')
janelaPrincipal.geometry('400x600')

textoPrincipal = Label(janelaPrincipal, text='Selecione a opção a ser executada')
textoPrincipal.grid(column=0,row=0, padx=10,pady=10)

botCaptura = Button(janelaPrincipal, text='Iniciar captura de dados',command=enviaDados)
botCaptura.grid(column=0,row=2, padx=10,pady=10)

status_text = Label(janelaPrincipal, text='')
status_text.grid(column=0,row=3, padx=10,pady=10)

janelaPrincipal.mainloop()