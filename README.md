to run this program you need to open command line and run the program in this way : ./runapp -d /home/nafise/Pictures/FTP_server/files/ -p 8080 -u 123 

instead of -d you can enter any directory path you wish 

after you run the server you can run the client using nc localhost PORT command 

then client needs to autheticate itself (user USERNAME PASSWORD )  . you can fine the list of usernames and passwords in repo under credentials.txt file 

if client is authorized he or she is allowed to use server facilities like ( list , get , put ) 

please enter all commands in lower case 

after put command my project is saving the content in case it faces ( . ) so at the end of your content please place dot .

