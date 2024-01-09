to compile this program you need write this command -> g++ project.cpp -o runapp
 
to run this program you need to open command line and run the program in this way -> ./runapp -d /home/nafise/Pictures/FTP_server/files/ -p 8080 -u 123 

NOTE : instead of -d you can enter any directory path you wish 

after you run the server you can run the client using nc localhost PORT command (in this example 8080)

then client needs to autheticate itself by entering this command -> user USERNAME PASSWORD .( you can find the list of usernames and passwords in repo under credentials.txt file )

if client is authorized he or she is allowed to use server facilities like ( list , get , put ) 

NOTE : please enter all commands in lower case 

after put command my project is saving the content in case it faces ( . ) so at the end of your content please place a dot .

github link -->  https://github.com/NafiseKian/FTP_server/
