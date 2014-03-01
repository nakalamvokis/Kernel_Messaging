all: testmailbox customtest
	
customtest: customtest1.o mailbox.o
	gcc customtest1.o mailbox.o -o customtest
	
testmailbox: testmailbox1.o mailbox.o
	gcc mailbox.o testmailbox1.o -o testmailbox
	
customtest1.o: customtest1.c mailbox.h
	gcc customtest1.c -c -g
	
testmailbox0.o: testmailbox0.c mailbox.h
	gcc testmailbox0.c -c -g
	
testmailbox1.o: testmailbox1.c mailbox.h
	gcc mailbox.h testmailbox1.c -c -g
	
mailbox.o: mailbox.c mailbox.h
	gcc mailbox.c mailbox.h -c

clean:
	rm *.o *.gch testmailbox
