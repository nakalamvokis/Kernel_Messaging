all: testmailbox1.o mailbox.o
	gcc mailbox.o testmailbox1.o -o testmailbox

testmailbox0.o: testmailbox0.c mailbox.h
	gcc testmailbox0.c -c -g
	
testmailbox1.o: testmailbox1.c mailbox.h
	gcc mailbox.h testmailbox1.c -c -g
	
mailbox.o: mailbox.c mailbox.h
	gcc mailbox.c mailbox.h -c

clean:
	rm *.o *.gch testmailbox
