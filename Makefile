
main: lex.yy.c
	g++ -o mss main.cpp getopt.c lex.yy.c -lfl

clean:
	rm -f *~ ./mss

install:
	cp -f ./mss /usr/bin

lex.yy.c: word.l
	flex -i word.l

