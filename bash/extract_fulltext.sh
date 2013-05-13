#!/bin/bash
# (c) Pierre Jourlin, Université d'Avignon, march 2010

if [ $# -ne 1 ]
then
        echo Usage : $0 sciencedirectfile
	echo produces sciencedirectfile.fulltext.html that contains only the article
else
        cat $1 | tr '\n' '\t' | sed 's:.*\(<div class="articleTitle">.*<div class="bottomArticle">\).*:\1:'|tr '\t' '\n' > $1.fulltext.html
fi


