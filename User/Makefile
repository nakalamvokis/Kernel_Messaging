all: testmailbox1 testmailbox2 testmailbox3 testmailbox4 testmailbox5 testmailbox7 customtest0 customtest1 customtest2

customtest0: customtest0.o mailbox.o
	gcc customtest0.o mailbox.o -o customtest0
	
customtest1: customtest1.o mailbox.o
	gcc customtest1.o mailbox.o -o customtest1

customtest2: customtest2.o mailbox.o
	gcc customtest2.o mailbox.o -o customtest2
	
testmailbox1: testmailbox1.o mailbox.o
	gcc mailbox.o testmailbox1.o -o testmailbox1
	
testmailbox2: testmailbox2.o mailbox.o
	gcc mailbox.o testmailbox2.o -o testmailbox2
	
testmailbox3: testmailbox3.o mailbox.o
	gcc mailbox.o testmailbox3.o -o testmailbox3
	
testmailbox4: testmailbox4.o mailbox.o
	gcc mailbox.o testmailbox4.o -o testmailbox4
	
testmailbox5: testmailbox5.o mailbox.o
	gcc mailbox.o testmailbox5.o -o testmailbox5
	
testmailbox6: testmailbox6.o mailbox.o
	gcc mailbox.o testmailbox6.o -o testmailbox6
	
testmailbox7: testmailbox7.o mailbox.o
	gcc mailbox.o testmailbox7.o -o testmailbox7
	
customtest0.o: customtest0.c mailbox.h
	gcc customtest0.c -c
	
customtest1.o: customtest1.c mailbox.h
	gcc customtest1.c -c
	
customtest2.o: customtest2.c mailbox.h
	gcc customtest2.c -c
	
testmailbox1.o: testmailbox1.c mailbox.h
	gcc mailbox.h testmailbox1.c -c

testmailbox2.o: testmailbox2.c mailbox.h
	gcc mailbox.h testmailbox2.c -c

testmailbox3.o: testmailbox3.c mailbox.h
	gcc mailbox.h testmailbox3.c -c
	
testmailbox4.o: testmailbox4.c mailbox.h
	gcc mailbox.h testmailbox4.c -c
	
testmailbox5.o: testmailbox5.c mailbox.h
	gcc mailbox.h testmailbox5.c -c
	
testmailbox6.o: testmailbox6.c mailbox.h
	gcc mailbox.h testmailbox6.c -c
	
testmailbox7.o: testmailbox7.c mailbox.h
	gcc mailbox.h testmailbox7.c -c
	
mailbox.o: mailbox.c mailbox.h
	gcc mailbox.c mailbox.h -c -g

clean:
	rm *.o *.gch testmailbox1 testmailbox2 testmailbox3 testmailbox4 testmailbox5 testmailbox7 customtest0 customtest1 customtest2
