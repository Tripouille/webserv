
CONTAINERS	= list vector map stack queue
BINS		= $(CONTAINERS:%=tests/ft::%Test) $(CONTAINERS:%=tests/std::%Test)
MAKE_RESULT	= $(CONTAINERS:%=ft\:\:%) $(CONTAINERS:%=std\:\:%)
RESULTS		= $(CONTAINERS:%=results/ft::%.result) $(CONTAINERS:%=results/std::%.result)
FT_SED		= $(CONTAINERS:%=ft\:\:%sed)
STD_SED		= $(CONTAINERS:%=std\:\:%sed)
FT_COLORS	= $(CONTAINERS:%=ft\:\:%color)
STD_COLORS	= $(CONTAINERS:%=std\:\:%color)

ifeq ($c, 1)
	CPPFLAGS   = -D CONST_TEST
endif

CC		= clang++
CFLAGS	= -g3 -Wall -Wextra -Werror -Wconversion -std=c++98 -I includes -I templates

all: $(CONTAINERS:%=diff\:\:%)

$(CONTAINERS:%=tests/ft\:\:%Test): tests/ft\:\:%: tests/%.cpp
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/$*.cpp -o $@

$(CONTAINERS:%=tests/std\:\:%Test): tests/std\:\:%: tests/%.cpp
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/$*.cpp -o $@

$(FT_SED): ft\:\:%sed:
	sed -ri s/"(test_$*<).*(>)"/"\1ft::$*\2"/ tests/$*Test.cpp

$(STD_SED): std\:\:%sed:
	sed -ri s/"(test_$*<).*(>)"/"\1std::$*\2"/ tests/$*Test.cpp

$(MAKE_RESULT): %: %color %sed tests/%Test
	valgrind -q --leak-check=full ./tests/$@Test > results/$@.result

$(MAKE_RESULT:%=show\:\:%): show\:\:%: %
	cat results/$<.result

$(CONTAINERS:%=diff\:\:%): diff\:\:%: std\:\:% ft\:\:%
	@tput setaf 3
	diff -I "#.*" -s --unified=0 results/ft::$*.result results/std::$*.result

$(CONTAINERS): %: std\:\:% ft\:\:%
	cat results/ft::$@.result
	@tput setaf 3
	diff -I "#.*" -s --unified=0 results/ft::$@.result results/std::$@.result

clean: color
	rm -rf $(BINS)

fclean:	clean
	rm -rf $(RESULTS)

re:	fclean all

$(FT_COLORS):
	@tput setaf 6

$(STD_COLORS):
	@tput setaf 4

color:
	@tput setaf 1

.PHONY:	all clean fclean re $(FT_COLORS) $(STD_COLORS) color