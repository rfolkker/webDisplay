#!/ c:\Program Files (x86)\Microsoft Visual Studio\Shared\Python37_64\python.exe
import requests
from time import sleep
requests.get('http://192.168.1.63/clear')
for count in range(100):
    data = {"txtpos":"64","progress":count,"text":"Shutting down","showPercent":"True"}
    # response = requests.get('http://192.168.1.63/clear')
    response = requests.post('http://192.168.1.63/progress', json = data)
    sleep(1)
requests.get('http://192.168.1.63/clear')
data = {"txtpos":"64","progress":"100","text":"Shutting down","showPercent":"True"}
response = requests.post('http://192.168.1.63/progress', json = data)
sleep(2)
requests.get('http://192.168.1.63/clear')
data = {"xpos":"0","ypos":"0","text":"Terminated."}
requests.post('http://192.168.1.63/text', json = data)