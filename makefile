BASENAME=cea
BASE=../$(BASENAME)
DOCDIR=Articles
DBHOST=localhost
DBUSER=hypolite
DBPASSWD=Hlaeem3f?
DBNAME=lbd$(BASENAME)
WWWRoot=/var/www/LBDNew/
NROWS=100
SSCL=2
GCCOPTS=-mtune=native -O3 -fomit-frame-pointer


all: CreateDataBase CreateBiblio $(BASE)/doclist.txt CreateEntities CreateHypothesis web/param.inc.php

clean :
	rm -f $(BASE)/Bibtex.Files/All.utf8.bib *~ ../bin/* cpp/*~ web/param.inc.php $(BASE)/doclist.txt

CreateDataBase : 
	createdb -h $(DBHOST) -U $(DBUSER) -W  $(DBNAME)
			 
./$(BASE)/ListUrls.txt : $(BASE)/Bibtex.Files
	   grep "http://www.sciencedirect.com/" $(BASE)/Bibtex.Files/*.bib | sort -u | sed 's:^.*\(http\):\1:' | sed 's/".*$///' | sort -u > ./$(BASE)/ListUrls.txt
 
$(BASE)/Bibtex.Files/All.utf8.bib : $(BASE)/Bibtex.Files/All.bib
	iconv --from-code=ISO-8859-1 --to-code=UTF-8 $(BASE)/Bibtex.Files/All.bib > $(BASE)/Bibtex.Files/All.utf8.bib

../bin/bib2sql : cpp/bib2sql.cpp
	  g++ cpp/bib2sql.cpp -o ../bin/bib2sql  $(GCCOPTS) -lpqxx

CreateBiblio : ../bin/bib2sql $(BASE)/Bibtex.Files/All.utf8.bib
	../bin/bib2sql $(DBHOST) $(DBUSER) $(DBPASSWD) $(DBNAME) $(BASE)/Bibtex.Files/All.utf8.bib 

$(BASE)/doclist.txt : $(BASE)/$(DOCDIR)
	find $(BASE)/$(DOCDIR) -type f -print > $(BASE)/doclist.txt

../bin/EntityExtractor : cpp/EntityExtractor.cpp
	  g++ cpp/EntityExtractor.cpp -o ../bin/EntityExtractor  $(GCCOPTS) -lboost_regex-mt  -lpqxx

CreateEntities : ../bin/EntityExtractor $(BASE)/doclist.txt
	../bin/EntityExtractor $(DBHOST) $(DBUSER) $(DBPASSWD) $(DBNAME) $(BASE)/doclist.txt 

../bin/LBDCoreSimpleStemCat : cpp/LBDCoreSimpleStemCat.cpp
	  g++ cpp/LBDCoreSimpleStemCat.cpp -o ../bin/LBDCoreSimpleStemCat  $(GCCOPTS) -lpqxx

CreateHypothesis : ../bin/LBDCoreSimpleStemCat 
	../bin/LBDCoreSimpleStemCat -l$(SSCL) -n$(NROWS) $(DBHOST) $(DBUSER) $(DBPASSWD) $(DBNAME)

CreateSemanticTables:
	psql -c "CREATE TABLE semcat(id SERIAL, name character varying(500) UNIQUE, mark integer DEFAULT 1 NOT NULL, CONSTRAINT betweenoneandfive CHECK (((mark >= 1) AND (mark <= 5))));" -W -h $(DBHOST) -d $(DBNAME) -U $(DBUSER)
	psql -c "CREATE TABLE isofsemcat(stcid integer, secid integer);" -W -h $(DBHOST) -d $(DBNAME) -U $(DBUSER)
	psql -c "CREATE TABLE taggerlog(ip character varying(200) NOT NULL, date date NOT NULL, time time with time zone NOT NULL, command character varying(200) NOT NULL);" -W -h $(DBHOST) -d $(DBNAME) -U $(DBUSER)

	
WWW	: web/param.inc.php
web/param.inc.php : ./web $(WWWRoot)
	echo "<?" > web/param.inc.php
	echo '$$host="'$(DBHOST)'";' >>web/param.inc.php 
	echo '$$user="'$(DBUSER)'";' >>web/param.inc.php 
	echo '$$mdp="'$(DBPASSWD)'";' >>web/param.inc.php 
	echo '$$dbname="'$(DBNAME)'";' >>web/param.inc.php 
	echo "?>">>web/param.inc.php 
	mkdir -p $(WWWRoot)/$(BASENAME)
	cp -r web/* $(WWWRoot)/$(BASENAME)





