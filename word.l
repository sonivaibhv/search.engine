%{
#include <stdio.h>

extern int add_word(char*);
%}

WORD [a-zA-Z0-9]+

%%
is |
the |
and |
as |
at |
but |
by |
down |
for |
from |
in |
into |
like |
of |
off |
on |
over |
per |
than |
to |
until |
up |
upon |
with {
	}
{WORD} {
	add_word(yytext);
	}
[^a-zA-Z0-9] {
	}
%%
