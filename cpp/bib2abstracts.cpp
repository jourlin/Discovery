/* Bib2SQL : Read a bibtex file (utf8), create table, insert fields

    Copyright (C) 2009  Pierre Jourlin

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************************
  Bib2SQL : Lit un fichier bibtex (utf8), crée la table, insère les champs

  Copyright (C) 2009 Pierre Jourlin — Tous droits réservés.
 
  Ce programme est un logiciel libre ; vous pouvez le redistribuer ou le
  modifier suivant les termes de la “GNU General Public License” telle que
  publiée par la Free Software Foundation : soit la version 3 de cette
  licence, soit (à votre gré) toute version ultérieure.
  
  Ce programme est distribué dans l’espoir qu’il vous sera utile, mais SANS
  AUCUNE GARANTIE : sans même la garantie implicite de COMMERCIALISABILITÉ
  ni d’ADÉQUATION À UN OBJECTIF PARTICULIER. Consultez la Licence Générale
  Publique GNU pour plus de détails.
  
  Vous devriez avoir reçu une copie de la Licence Générale Publique GNU avec
  ce programme ; si ce n’est pas le cas, consultez :
  <http://www.gnu.org/licenses/>.

    Pierre Jourlin
    L.I.A. / C.E.R.I.
    339, chemin des Meinajariès
    BP 1228 Agroparc
    84911 AVIGNON CEDEX 9
    France 
    pierre.jourlin@univ-avignon.fr
    Tel : +33 4 90 84 35 32
    Fax : +33 4 90 84 35 01

*/

#define LINEMAXLENGTH 20000
#define KEYMAXLENGTH 500

#include <iostream>
#include <fstream>
#include <map>
#include <stdlib.h>
#include <string.h>

#include <pqxx/connection>
#include <pqxx/transaction>


using namespace std;
using namespace pqxx;
using namespace PGSTD;

char cc;
char CurrentID[KEYMAXLENGTH];

char line[LINEMAXLENGTH];
char Identifier[LINEMAXLENGTH];
char FieldName[LINEMAXLENGTH];
char FieldValue[LINEMAXLENGTH];

char sqlqry[LINEMAXLENGTH];

map <string, string> Entry;
map <string, string>::const_iterator iter;

unsigned long int nrows=1;
unsigned int pos=0;

ifstream bibfile;
ofstream abstfile;

void TransformQuotes(char *str)
{
	char *c=str;
	while(*c!=(char) 0)
	{
		if(*c=='"' || *c=='\'')
			*c='/';
		c++;
	}
}
void Read(void)
{
  
  if((cc==(char) 10)|| (cc==(char) 13)||(cc=='\n')||(cc==(char) 0))
	    {
	      nrows++;
	      pos=0;
	      bibfile.getline(line, LINEMAXLENGTH);
	     }
  cc=line[pos++];
}

void ReadWhiteChar(void)
{
  while(cc==(char) 0 || cc==' '||cc=='\n'||cc=='\t'||cc== (char) 13 )
    {
	Read();
    }
}

void ReadKeyWord(const char *str)
{
  char word[KEYMAXLENGTH], *tmp=word, *test=(char *) str;
  
  while( *test!='\0' && tolower(*test)==tolower(cc))
    {
      *tmp++=cc;
      Read();
      test++;
    }
  *tmp='\0';
  if(*test!=0)
    {
      cerr << "Keyword "<<str<<" expected at line "<< nrows <<", read : "<<word<<endl;
      exit(-1);
    }
}


void ReadField(char *word)
{
  char *tmp=word;
  
  while(cc!=' ' && cc!='\n' && cc!='\013' && cc!='\t' && cc !='=')
    {
      *tmp++=cc;
      Read();
    };
    *tmp='\0';
}

void ReadString(char *word)
{
	char *tmp=word;
	do
	{
		
		while(cc!='"')
		{
			if( (cc!=(char) 0) &&(cc!=(char) 10) && (cc != (char) 13) )
				*tmp++=cc;
     	 		Read();
		};
		Read();
		if(cc==(char) 13)
			break;
		
	} 
	while((cc!=(char) 0) && (cc!=',' || ( 
						(line[pos]!=(char) 0) && 
						(line[pos]!=(char) 10) && 
						(line[pos]!=(char) 13) 
					      ) ));
 	*tmp='\0';

 	TransformQuotes(word);
}

void ReadID(char *word)
{
  char *tmp=word;
  
  while(cc!=' ' && cc!='\n' && cc!='\t' && cc !=',')
    {
      *tmp++=cc;
      Read();	
    }
    *tmp='\0';
	TransformQuotes(word);
    strcpy(CurrentID, word);
}

int main(int argc, char **argv)
{
	unsigned long long int nbart=0, c;
	string insertstr;
	string pgconfig;

	if(argc!=6)
	{
		cerr<<"Usage : "<<argv[0]<<" bdhost bduser bdpassword bdname bibtexfile"<<endl;
		exit(-1);
	}
	bibfile.open(argv[5]);
	
	pgconfig="host="+string(argv[1])+" port="+"5432"+" dbname="+string(argv[4])+" user="+string(argv[2])+" password="+string(argv[3]);
  	connection pgcon(pgconfig.c_str());
	try
    	{
      		work Drop(pgcon, "drop_Bibentries");
      		disable_noticer d(pgcon);
      	Drop.exec("DROP TABLE Bibentries");
      	Drop.commit();
    	}
    	catch (const undefined_table &e)
    	{
      		cout << "(Expected) Couldn't drop table: " << e.what() << endl
	   	<< "Query was: " << e.query() << endl;
    	}
    	catch (const sql_error &e)
    	{
      		cerr << "Couldn't drop table: " << e.what() << endl
	   	<< "Query was: " << e.query() << endl;
    	}
	try
    	{
      		work Create(pgcon, "create_Bibentries");
      		disable_noticer d(pgcon);
      		Create.exec("CREATE TABLE Bibentries (\
			  title text   NULL,\
			  author text   NULL,\
			  journal varchar(200)   NULL,\
			  year varchar(10)   NULL,\
			  abstract text   NULL,\
			  url varchar(500)  NULL,\
			  volume varchar(50)   NULL,\
			  number varchar(50)   NULL,\
			  pages varchar(60)  NULL,\
			  note text   NULL,\
			  issn varchar(100)   NULL,\
			  doi varchar(100)   NULL,\
			  key varchar(100)  NULL,\
			  keywords text   NULL,\
			  id varchar(100)   NULL)");
      		Create.commit();
		nrows=0;
  		pos=0;
  		bibfile.getline(line, LINEMAXLENGTH);
 		cc=line[pos++];
 		ReadWhiteChar();

 		while(cc=='@')	// Process the current entry
  		{
    			Read();
    			ReadKeyWord("article");
    			ReadWhiteChar();
    			ReadKeyWord("{");
    			ReadWhiteChar();
    			ReadID(Identifier);
			//cerr << "Identifier :"<<Identifier<<", ";
    			ReadWhiteChar();
    			while(cc==',')
    			{
      				ReadKeyWord(",");
      				ReadWhiteChar(); 
				ReadField(FieldName);
				//cerr << FieldName<<" => ";
				ReadWhiteChar();
				ReadKeyWord("=");
				ReadWhiteChar();
				ReadKeyWord("\"");
				ReadString(FieldValue);
				//cerr << FieldValue<<endl;
				ReadWhiteChar();
				Entry[FieldName]=FieldValue;
			}
			ReadKeyWord("}");
			ReadWhiteChar();
			if(Entry.find("abstract")!=Entry.end() && Entry.find("url")!=Entry.end())
				{
					abstfile.open(Entry["url"].substr(Entry["url"].find_last_of("/")+1).c_str());
					abstfile << Entry["abstract"];
					abstfile.close();
					//cerr << Entry["url"].substr(Entry["url"].find_last_of("/")+1)<<endl<<Entry["abstract"]<<endl;
				}
				
			insertstr="INSERT INTO Bibentries (";
			iter=Entry.begin();
			insertstr+=""+iter->first+"";
			iter++;
			for(;iter!=Entry.end();++iter)
			{
				insertstr+=","+iter->first+"";
			}
			insertstr+=") VALUES (";
			iter=Entry.begin();
			insertstr+="'"+iter->second+"'";
			iter++;
			for(;iter!=Entry.end();++iter)
			{
				insertstr+=",'"+iter->second+"'";
			}
			insertstr+=");";
			for(c=0; c<insertstr.size();c++)
				sqlqry[c]=insertstr[c];
			sqlqry[c]='\0';
			try
    			{
      				work Insert(pgcon, "insert_Bibentries");
      				disable_noticer d(pgcon);
      				Insert.exec(sqlqry);
      				Insert.commit();
    			}
			// Failed to insert
    			catch (const undefined_table &e)
    			{
      				cout << "(Expected) Couldn't insert into table: " << e.what() << endl
		   		<< "Query was: " << e.query() << endl;
				return -1;	// No use continuing
    			}
    			catch (const sql_error &e)
    			{
      				cerr << "Couldn't insert into table: " << e.what() << endl
		   		<< "Query was: " << e.query() << endl;
				return -1; // No use continuing
    			}
			Entry.clear();
			cerr << "Processed document number "<<++nbart<<"      "<<(char) 13;
		}// All entries are processed

		bibfile.close();
		cerr<<endl;
    	}
	// Failed to create table Bibentries
    	catch (const undefined_table &e)
    	{
      		cout << "(Expected) Couldn't create table: " << e.what() << endl
	   	<< "Query was: " << e.query() << endl;
    	}
    	catch (const sql_error &e)
    	{
      		cerr << "Couldn't create table: " << e.what() << endl
	   	<< "Query was: " << e.query() << endl;
    	}
	
  	
  	return 0;
}
