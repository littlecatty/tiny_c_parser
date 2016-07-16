all:tiny_c_parser.c
	gcc tiny_c_parser.c -o tiny_c_parser
clean: tiny_c_parser
	rm tiny_c_parser	