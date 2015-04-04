install: epub2pdf html2pdf
	install -m 0755 ./epub2pdf ~/.local/bin/epub2pdf
	install -m 0755 ./html2pdf ~/.local/bin/html2pdf

epub2pdf: epub2pdf.d
	gdc -O2 -g -o $@ $^

html2pdf: html2pdf.c
	gcc -O2 -g -o $@ $^ `pkg-config --libs --cflags webkit2gtk-4.0`
