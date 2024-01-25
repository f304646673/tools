
BEGIN {
  unless ($ENV{AUTHOR_TESTING}) {
    print qq{1..0 # SKIP these tests are for testing by the author\n};
    exit
  }
}

use strict;
use warnings;
use Test::More;

use Test::Spelling;
use Pod::Wordlist;

add_stopwords(<DATA>);
all_pod_files_spelling_ok( qw( bin lib ) );

__DATA__
ACC
ACCM
al
AMLOC
analized
analizo
Analizo
Andreas
ANPM
Araujo
bd
cardinality
Carliss
CBO
Chidamber
codigo
Costa
CPUs
csv
cxx
cyclomatic
Cyclomatic
da
DIT
doxyparse
Doxyparse
dsm
DSN
ecfb
efd
egypt
fbad
globalonly
GPL
Graphviz
Gratio
Guerreiro
Gustafsson
Hedley
Hennell
hh
hpp
Hyatt
Joao
Joenio
Kemerer
Kessler
Khoshgoftaar
kurtosis
LCOM
LOC
MacCormack
McCabe
Meirelles
metricas
Moreira
Munson
myproject
neato
NOA
NOC
NOM
NPA
NPM
Paulo
pgdb
Pimenta
Piveta
PNG
PostScript
progressbar
relicensed
relicensing
Rusnak
Shyam
skewness
Soares
src
Taghi
Terceiro
undirected
Monitoramento
código
de
fonte
livre
métricas
projetos
Marques
SC
cc
cf
java
db
