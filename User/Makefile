all: testmailbox1 customtest0 customtest1

customtest0: customtest0.o mailbox.o
	gcc customtest0.o mailbox.o -o customtest0
	
customtest1: customtest1.o mailbox.o
	gcc customtest1.o mailbox.o -o customtest1

testmailbox1: testmailbox1.o mailbox.o
	gcc mailbox.o testmailbox1.o -o testmailbox1
	
customtest0.o: customtest0.c mailbox.h
	gcc customtest0.c -c
	
customtest1.o: customtest1.c mailbox.h
	gcc customtest1.c -c
	
testmailbox1.o: testmailbox1.c mailbox.h
	gcc mailbox.h testmailbox1.c -c
	
mailbox.o: mailbox.c mailbox.h
	gcc mailbox.c mailbox.h -c -g

clean:
	rm *.o *.gch testmailbox1 customtest0 customtest1
