"""
@author         : "Alessandro Mannini"
@organization   : "Università degli Studi di Firenze"
@contact        : "alessandro.mannini@gmail.com"
@date           : "Jan 10, 2023"
@version        : "1.0.0"
"""
import PySimpleGUI as sg
from  model.layout import Layout
from model.node import Node
import view.main as View
import controller.main as Controller

class App():
    def __init__(self) -> None:
        # Instantiate model, view and controller
        self.__view: View.Main = View.Main()
        self.__model: Layout = Layout('', '')
        self.__controller: Controller.Main = Controller.Main(self.__model, self.__view)

        # Wait for message until close
        self.__view.eventLoop()
                
if __name__ == '__main__':
    app = App()
