# dropper_app

The application you need (* .exe) needs to be translated into an array and written to the xor_array.h header file.</br>
This is done using a Python script bin2array.py.</br>
Example: python bin2array.py app.exe.</br>
The output will be the file xor_array.h.</br>
Next, you need to compile the project: dropper_app.cpp, xor_array.h.</br>
When you run the compiled dropper_app.exe, * .exe is created from the resulting array.</br>
The resulting * .exe is written to the script (Windwos Script), and this script is automatically launched in the Windows registry.</br>
</br></br>
Нужное вам приложение (*.exe) нужно перевести в массив и записать его в заголовочный файл xor_array.h.</br>
Делается это при помощи скрипта на Python bin2array.py.</br>
Пример: python bin2array.py app.exe.</br>
На выходе получим файл xor_array.h.</br>
Далее нужно скомпилировать проект: dropper_app.cpp, xor_array.h.</br>
При запуске скомпилированного dropper_app.exe происходит создание *.exe из полученного массива.</br>
Полученное *.exe прописывается в скрипт (Windwos Script), а этот скрипт в автозапуск в реестр Windows.</br>
