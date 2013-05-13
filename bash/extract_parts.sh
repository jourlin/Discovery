#!/bin/bash
# (c) Pierre Jourlin, Université d'Avignon, march 2010
# when 1st argument is a sciencedirect html file that contains a full text article, it produces :
# - sciencedirectfile.fulltext.html that contains only article+abstract+ref
# - sciencedirectfile.abstract.html that contains only the abstract
# 
# recursively apply to files using 'find', e.g :
# find ../../inra/Articles/ -type f -exec ./extract_fulltext.sh {} \;
# 

if [ $# -ne 1 ]
then
        echo Usage : $0 sciencedirectfile
else   
	
	if [ `grep ">Article Outline</h3>" $1 |wc -l`  -ge 1 ]
	then
		cat $1 | tr '\n' '\t' | sed 's:.*\(<div class="articleTitle">.*<div class="bottomArticle">\).*:\1:'|tr '\t' '\n' > $1.fulltext.html
		cat $1.fulltext.html | tr '\n' '\t' | sed 's:.*<h3 class="h3">Abstract</h3><p>::'|sed 's:</p>.*::' |tr '\t' '\n' > $1.abstract.html
	fi
fi


