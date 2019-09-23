
# Описание протокола

Соединение:

 ```
Client					Server
|	---------->			|
|	SESSION_ID, 0			|
|					|
|	<----------			|
|	SESION_ID, <номер_сессии>	|
 ```

Получение списка файлов:

 ```
Client								Server
|	---------->						|
|	GET_FILE_LIST						|
|								|
|	<----------						|
|	FILE_LIST, <длина_строки>, <файл1\n>, <файл2\n>...	|
 ```
 
Получение файла:

 ```
Client							Server
|	---------->					|
|	GET_FILE, <число_соединений>, <имя_файла>	|
|							|
|	<----------					|
|	OPEN_CONN, <число_соединений>, <размер_файла>	|
|							|
||||	=========>>					||||
||||	SESION_ID, <номер_сессии>			||||
||||	<<=========					||||
||||	<файл>						||||
|							|
 ```
