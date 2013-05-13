/* EntityExtractor.cpp : Extract the definitions of acronyms that occurs in a html or ascii text file

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
  EntityExtractor.cpp : Extrait les définition d'accronymes présentes dans un fichier texte html ou ascii

  Copyright (C) 2009 Pierre Jourlin — Tous droits réservés.
 
  Ce programme est un logiciel libre ; vous pouvez le redistribuer ou le
  modifier suivant les termes de la "GNU General Public License" telle que
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

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <map>
#include <set>
#include <boost/regex.hpp>
#include <pqxx/connection>
#include <pqxx/transaction>
#include "OleanderStemming/english_stem.h"

#define MAXSIZEFORSTRINGS    10000		// Max filename length
#define MAXSTEMS		20		// Max number of stems per entity

using namespace std;
using namespace pqxx;
using namespace PGSTD;

void DropTables(connection& pgcon)
{
	string TableNames[5]={"Entities", "Documents", "Occurrences","StemSeqCat", "StemSeqCatContain"};
	for(int i=0; i<5; i++)
	{	try
    		{
      			work Drop(pgcon, "DROP_"+TableNames[i]);
      			disable_noticer d(pgcon);
      			Drop.exec("DROP TABLE "+TableNames[i]);
      			Drop.commit();
    		}	
    		catch (const undefined_table &e)
    		{
      			cout << "[Warning] Couldn't create undefined table: " << e.what() << endl
			<< "Query was: " << e.query() << endl;
    		}
    		catch (const sql_error &e)
    		{
			throw e;
		}
	}	
}

void CreateTables(connection& pgcon)
{
	string TableNames[5]={"Entities", "Documents", "Occurrences","StemSeqCat", "StemSeqCatContain"};
	string TableQry[5];
	TableQry[0]="CREATE TABLE Entities (entid bigint NOT NULL, definition varchar(200) NOT NULL, initials varchar(100) NOT NULL)";
	TableQry[1]="CREATE TABLE Occurrences (entid bigint NOT NULL, docid bigint  NOT NULL)";
	TableQry[2]="CREATE TABLE Documents (docid bigint NOT NULL, docname varchar(100) NOT NULL)";
	TableQry[3]="CREATE TABLE StemSeqCat (scid bigint NOT NULL, length bigint NOT NULL, string varchar(200) NOT NULL)";
	TableQry[4]="CREATE TABLE StemSeqCatContain (scid bigint NOT NULL, entid bigint NOT NULL)";
	for(int i=0; i<5; i++)
	{
		try
		{
	      		work Create(pgcon, "create_"+TableNames[i]);
	      		disable_noticer d(pgcon);
	      		Create.exec(TableQry[i]);
	      		Create.commit();
		}
		//failed to create table
  		catch (const sql_error &e)
		{
			throw(e);
   		}
	}
}

char TranscodeChar(string source)
{
	static map<string, char> transcode;
	// Initialisation 
	if(transcode.find("&#x21;")==transcode.end())
	{
		transcode["&#x21;"]='!';
		transcode["&#x22;"]='"';
		transcode["&#x23;"]='#';
		transcode["&#x24;"]='$';
		transcode["&#x25;"]='%';
		transcode["&#x26;"]='&';
		transcode["&#x27;"]='\'';
		transcode["&#x28;"]='(';
		transcode["&#x29;"]=')';
		transcode["&#x2a;"]='*';
		transcode["&#x2b;"]='+';
		transcode["&#x2c;"]=',';
		transcode["&#x2d;"]='-';
		transcode["&#x2e;"]='.';
		transcode["&#x2f;"]='/';
		transcode["&#x30;"]='0';
		transcode["&#x31;"]='1';
		transcode["&#x32;"]='2';
		transcode["&#x33;"]='3';
		transcode["&#x34;"]='4';
		transcode["&#x35;"]='5';
		transcode["&#x36;"]='6';
		transcode["&#x37;"]='7';
		transcode["&#x38;"]='8';
		transcode["&#x39;"]='9';
		transcode["&#x3a;"]=':';
		transcode["&#x3b;"]=';';
		transcode["&#x3c;"]='<';
		transcode["&#x3d;"]='=';
		transcode["&#x3e;"]='>';
		transcode["&#x3f;"]='?';
		transcode["&#x40;"]='@';
		transcode["&#x41;"]='A';
		transcode["&#x42;"]='B';
		transcode["&#x43;"]='C';
		transcode["&#x44;"]='D';
		transcode["&#x45;"]='E';
		transcode["&#x46;"]='F';
		transcode["&#x47;"]='G';
		transcode["&#x48;"]='H';
		transcode["&#x49;"]='I';
		transcode["&#x4a;"]='J';
		transcode["&#x4b;"]='K';
		transcode["&#x4c;"]='L';
		transcode["&#x4d;"]='M';
		transcode["&#x4e;"]='N';
		transcode["&#x4f;"]='O';
		transcode["&#x50;"]='P';
		transcode["&#x51;"]='Q';
		transcode["&#x52;"]='R';
		transcode["&#x53;"]='S';
		transcode["&#x54;"]='T';
		transcode["&#x55;"]='U';
		transcode["&#x56;"]='V';
		transcode["&#x57;"]='W';
		transcode["&#x58;"]='X';
		transcode["&#x59;"]='Y';
		transcode["&#x5a;"]='Z';
		transcode["&#x5b;"]='[';
		transcode["&#x5c;"]='\\';
		transcode["&#x5d;"]=']';
		transcode["&#x5e;"]='^';
		transcode["&#x5f;"]='_';
		transcode["&#x60;"]='`';
		transcode["&#x61;"]='a';
		transcode["&#x62;"]='b';
		transcode["&#x63;"]='c';
		transcode["&#x64;"]='d';
		transcode["&#x65;"]='e';
		transcode["&#x66;"]='f';
		transcode["&#x67;"]='g';
		transcode["&#x68;"]='h';
		transcode["&#x69;"]='i';
		transcode["&#x6a;"]='j';
		transcode["&#x6b;"]='k';
		transcode["&#x6c;"]='l';
		transcode["&#x6d;"]='m';
		transcode["&#x6e;"]='n';
		transcode["&#x6f;"]='o';
		transcode["&#x70;"]='p';
		transcode["&#x71;"]='q';
		transcode["&#x72;"]='r';
		transcode["&#x73;"]='s';
		transcode["&#x74;"]='t';
		transcode["&#x75;"]='u';
		transcode["&#x76;"]='v';
		transcode["&#x77;"]='w';
		transcode["&#x78;"]='x';
		transcode["&#x79;"]='y';
		transcode["&#x7a;"]='z';
		transcode["&#x7b;"]='{';
		transcode["&#x7c;"]='|';
		transcode["&#x7d;"]='}';
		transcode["&#x7e;"]='~';
		transcode["&#x7f;"]='';
		transcode["&#x80;"]=' ';
		transcode["&#x81;"]=' ';
		transcode["&#x82;"]=' ';
		transcode["&#x83;"]=' ';
		transcode["&#x84;"]=' ';
		transcode["&#x85;"]=' ';
		transcode["&#x86;"]=' ';
		transcode["&#x87;"]=' ';
		transcode["&#x88;"]=' ';
		transcode["&#x89;"]=' ';
		transcode["&#x8a;"]=' ';
		transcode["&#x8b;"]=' ';
		transcode["&#x8c;"]=' ';
		transcode["&#x8d;"]=' ';
		transcode["&#x8e;"]=' ';
		transcode["&#x8f;"]=' ';
		transcode["&#x90;"]=' ';
		transcode["&#x91;"]=' ';
		transcode["&#x92;"]=' ';
		transcode["&#x93;"]=' ';
		transcode["&#x94;"]=' ';
		transcode["&#x95;"]=' ';
		transcode["&#x96;"]=' ';
		transcode["&#x97;"]=' ';
		transcode["&#x98;"]=' ';
		transcode["&#x99;"]=' ';
		transcode["&#x9a;"]=' ';
		transcode["&#x9b;"]=' ';
		transcode["&#x9c;"]=' ';
		transcode["&#x9d;"]=' ';
		transcode["&#x9e;"]=' ';
		transcode["&#x9f;"]=' ';
		transcode["&#xa0;"]=' ';
		transcode["&#xa1;"]=' ';
		transcode["&#xa2;"]=' ';
		transcode["&#xa3;"]=' ';
		transcode["&#xa4;"]=' ';
		transcode["&#xa5;"]=' ';
		transcode["&#xa6;"]=' ';
		transcode["&#xa7;"]=' ';
		transcode["&#xa8;"]=' ';
		transcode["&#xa9;"]=' ';
		transcode["&#xaa;"]=' ';
		transcode["&#xab;"]=' ';
		transcode["&#xac;"]=' ';
		transcode["&#xad;"]=' ';
		transcode["&#xae;"]=' ';
		transcode["&#xaf;"]=' ';
		transcode["&#xb0;"]=' ';
		transcode["&#xb1;"]=' ';
		transcode["&#xb2;"]=' ';
		transcode["&#xb3;"]=' ';
		transcode["&#xb4;"]=' ';
		transcode["&#xb5;"]=' ';
		transcode["&#xb6;"]=' ';
		transcode["&#xb7;"]=' ';
		transcode["&#xb8;"]=' ';
		transcode["&#xb9;"]=' ';
		transcode["&#xba;"]=' ';
		transcode["&#xbb;"]=' ';
		transcode["&#xbc;"]=' ';
		transcode["&#xbd;"]=' ';
		transcode["&#xbe;"]=' ';
		transcode["&#xbf;"]=' ';
		transcode["&#xc0;"]='A';
		transcode["&#xc1;"]='A';
		transcode["&#xc2;"]='A';
		transcode["&#xc3;"]='A';
		transcode["&#xc4;"]='A';
		transcode["&#xc5;"]='A';
		transcode["&#xc6;"]='A';
		transcode["&#xc7;"]='C';
		transcode["&#xc8;"]='E';
		transcode["&#xc9;"]='E';
		transcode["&#xca;"]='E';
		transcode["&#xcb;"]='E';
		transcode["&#xcc;"]='I';
		transcode["&#xcd;"]='I';
		transcode["&#xce;"]='I';
		transcode["&#xcf;"]='I';
		transcode["&#xd0;"]='D';
		transcode["&#xd1;"]='N';
		transcode["&#xd2;"]='O';
		transcode["&#xd3;"]='O';
		transcode["&#xd4;"]='O';
		transcode["&#xd5;"]='O';
		transcode["&#xd6;"]='O';
		transcode["&#xd7;"]='x';
		transcode["&#xd8;"]=(char) L'Ø';
		transcode["&#xd9;"]='U';
		transcode["&#xda;"]='U';
		transcode["&#xdb;"]='U';
		transcode["&#xdc;"]='U';
		transcode["&#xdd;"]='Y';
		transcode["&#xde;"]=(char) L'Þ';
		transcode["&#xdf;"]=(char) L'ß';
		transcode["&#xe0;"]='a';
		transcode["&#xe1;"]='a';
		transcode["&#xe2;"]='a';
		transcode["&#xe3;"]='a';
		transcode["&#xe4;"]='a';
		transcode["&#xe5;"]='a';
		transcode["&#xe6;"]='a';
		transcode["&#xe7;"]='c';
		transcode["&#xe8;"]='e';
		transcode["&#xe9;"]='e';
		transcode["&#xea;"]='e';
		transcode["&#xeb;"]='e';
		transcode["&#xec;"]='i';
		transcode["&#xed;"]='i';
		transcode["&#xee;"]='i';
		transcode["&#xef;"]='i';
		transcode["&#xf0;"]='o';
		transcode["&#xf1;"]='n';
		transcode["&#xf2;"]='o';
		transcode["&#xf3;"]='o';
		transcode["&#xf4;"]='o';
		transcode["&#xf5;"]='o';
		transcode["&#xf6;"]='o';
		transcode["&#xf7;"]=(char) L'÷';
		transcode["&#xf8;"]=(char) L'ø';
		transcode["&#xf9;"]='u';
		transcode["&#xfa;"]='u';
		transcode["&#xfb;"]='u';
		transcode["&#xfc;"]='u';
		transcode["&#xfd;"]='y';
		transcode["&#xfe;"]=(char) L'þ';
		transcode["&#xff;"]=(char) L'ÿ';
		transcode["&#x100;"]=(char) L'Ā';
		transcode["&#x101;"]=(char) L'ā';
		transcode["&#x102;"]=(char) L'Ă';
		transcode["&#x103;"]=(char) L'ă';
		transcode["&#x104;"]=(char) L'Ą';
		transcode["&#x105;"]=(char) L'ą';
		transcode["&#x106;"]=(char) L'Ć';
		transcode["&#x107;"]=(char) L'ć';
		transcode["&#x108;"]=(char) L'Ĉ';
		transcode["&#x109;"]=(char) L'ĉ';
		transcode["&#x10a;"]=(char) L'Ċ';
		transcode["&#x10b;"]=(char) L'ċ';
		transcode["&#x10c;"]=(char) L'Č';
		transcode["&#x10d;"]=(char) L'č';
		transcode["&#x10e;"]=(char) L'Ď';
		transcode["&#x10f;"]=(char) L'ď';
		transcode["&#x110;"]=(char) L'Đ';
		transcode["&#x111;"]=(char) L'đ';
		transcode["&#x112;"]=(char) L'Ē';
		transcode["&#x113;"]=(char) L'ē';
		transcode["&#x114;"]=(char) L'Ĕ';
		transcode["&#x115;"]=(char) L'ĕ';
		transcode["&#x116;"]=(char) L'Ė';
		transcode["&#x117;"]=(char) L'ė';
		transcode["&#x118;"]=(char) L'Ę';
		transcode["&#x119;"]=(char) L'ę';
		transcode["&#x11a;"]=(char) L'Ě';
		transcode["&#x11b;"]=(char) L'ě';
		transcode["&#x11c;"]=(char) L'Ĝ';
		transcode["&#x11d;"]=(char) L'ĝ';
		transcode["&#x11e;"]=(char) L'Ğ';
		transcode["&#x11f;"]=(char) L'ğ';
		transcode["&#x120;"]=(char) L'Ġ';
		transcode["&#x121;"]=(char) L'ġ';
		transcode["&#x122;"]=(char) L'Ģ';
		transcode["&#x123;"]=(char) L'ģ';
		transcode["&#x124;"]=(char) L'Ĥ';
		transcode["&#x125;"]=(char) L'ĥ';
		transcode["&#x126;"]=(char) L'Ħ';
		transcode["&#x127;"]=(char) L'ħ';
		transcode["&#x128;"]=(char) L'Ĩ';
		transcode["&#x129;"]=(char) L'ĩ';
		transcode["&#x12a;"]=(char) L'Ī';
		transcode["&#x12b;"]=(char) L'ī';
		transcode["&#x12c;"]=(char) L'Ĭ';
		transcode["&#x12d;"]=(char) L'ĭ';
		transcode["&#x12e;"]=(char) L'Į';
		transcode["&#x12f;"]=(char) L'į';
		transcode["&#x130;"]=(char) L'İ';
		transcode["&#x131;"]=(char) L'ı';
		transcode["&#x132;"]=(char) L'Ĳ';
		transcode["&#x133;"]=(char) L'ĳ';
		transcode["&#x134;"]=(char) L'Ĵ';
		transcode["&#x135;"]=(char) L'ĵ';
		transcode["&#x136;"]=(char) L'Ķ';
		transcode["&#x137;"]=(char) L'ķ';
		transcode["&#x138;"]=(char) L'ĸ';
		transcode["&#x139;"]=(char) L'Ĺ';
		transcode["&#x13a;"]=(char) L'ĺ';
		transcode["&#x13b;"]=(char) L'Ļ';
		transcode["&#x13c;"]=(char) L'ļ';
		transcode["&#x13d;"]=(char) L'Ľ';
		transcode["&#x13e;"]=(char) L'ľ';
		transcode["&#x13f;"]=(char) L'Ŀ';
		transcode["&#x140;"]=(char) L'ŀ';
		transcode["&#x141;"]=(char) L'Ł';
		transcode["&#x142;"]=(char) L'ł';
		transcode["&#x143;"]=(char) L'Ń';
		transcode["&#x144;"]=(char) L'ń';
		transcode["&#x145;"]=(char) L'Ņ';
		transcode["&#x146;"]=(char) L'ņ';
		transcode["&#x147;"]=(char) L'Ň';
		transcode["&#x148;"]=(char) L'ň';
		transcode["&#x149;"]=(char) L'ŉ';
		transcode["&#x14a;"]=(char) L'Ŋ';
		transcode["&#x14b;"]=(char) L'ŋ';
		transcode["&#x14c;"]=(char) L'Ō';
		transcode["&#x14d;"]=(char) L'ō';
		transcode["&#x14e;"]=(char) L'Ŏ';
		transcode["&#x14f;"]=(char) L'ŏ';
		transcode["&#x150;"]=(char) L'Ő';
		transcode["&#x151;"]=(char) L'ő';
		transcode["&#x152;"]=(char) L'Œ';
		transcode["&#x153;"]=(char) L'œ';
		transcode["&#x2019;"]='\'';
		transcode["\302\264"]='e';
                transcode["\302\250"]='u'; // ü
                transcode["\302\277"]='f'; // fi
                transcode["\302\264"]='o'; // o'
                transcode["\304\261"]='r'; // r-
                transcode["\305\257"]='u'; // °u
		transcode["\313\234"]='-'; // r-t end-of-column hyphen 
		transcode["\316\263"]='y'; // -y
                transcode["\317\203"]='-'; // end-of-column hyphen
		transcode["\342\200\224"]='\'';
		transcode["\342\200\231"]='\'';
		transcode["\342\200\234"]='\'';
		transcode["\342\200\267"]=' ';
		transcode["\342\200\240"]=' ';
		transcode["\342\210\222"]='-';
		transcode["\357\254\201"]='f'; // fi
		transcode["\357\254\202"]='f'; // fl
		transcode["\357\243\275"]=' '; // ?

	}
	else
		if(transcode.find(source)!=transcode.end())
			return transcode[source];
		else 
			return '?';
}

string Transcode(string source)
{
	string transcoded;
	static boost::regex expression( "(&#x[0-9a-f]+;)|(\302.)|(\304.)|(\305.)|(\313.)|(\316.)|(\317.)|(\342\200.)|(\342\210\222)|(\357..)" );
	string::const_iterator start, end, iter; 
      	boost::match_results<std::string::const_iterator> what; 
   	boost::match_flag_type flags = boost::match_default; 

	start = source.begin(); 
   	end = source.end(); 
	
      	while(regex_search(start, end, what, expression, flags)) 
  	{ 
		transcoded+=string(start, what[0].first);
		transcoded.push_back(TranscodeChar(string(what[0].first, what[0].second)));
      		start = what[0].second; 
      		// update flags: 
      		flags |= boost::match_prev_avail; 
      		flags |= boost::match_not_bob;
   	} 
	transcoded+=string(start, end);

	return transcoded;
}

string EscapeQuotes(string source)
{
	int pos=0;
	while(pos < source.size() && ((pos=source.find("'", pos))!=string::npos))
    	{
		source.replace(pos, 1, "\\'");
		pos+=2;
	}
	return source;
}

bool isaword(char *word)
{
	if((*word>='A')&&(*word<='Z')) 	// If the word starts with a capital letter,
		*word=tolower(*word);	// turn this letter into lowercase.	
	while((*word>='a')&&(*word<='z'))
		word++;
	return (*word)=='\0';
}

bool isanacronym(char *word)
{
	int nbletters=0;
	while((*word>='A')&&(*word<='Z'))
	{
		word++;
		nbletters++;
	};
	return (nbletters>=2) && ((*word)=='\0');
}

char Disaccent(char c)
{
	static string accent="ÀÁÂÃÄÅàáâãäåÒÓÔÕÖØòóôõöøÈÉÊËèéêëÌÍÎÏìíîïÙÚÛÜùúûüÿÑñÇç";
	static string noaccent= "AAAAAAaaaaaaOOOOOOooooooEEEEeeeeIIIIiiiiUUUUuuuuyNnCc"; 
	if(accent.find_first_of(c)!=-1)
		return noaccent[accent.find_first_of(c)];
	else 
		return c;	
}

string wstring2string(wstring wstr) 
{
	string str(wstr.length(),' ');
	copy(wstr.begin(),wstr.end(),str.begin());
	return str;
}

wstring string2wstring(string str) 
{
	wstring wstr(str.length(),L' ');
	copy(str.begin(),str.end(),wstr.begin());
	return wstr;
}

void MRTS(connection& pgcon, unsigned long long sourceid, string source)	// transform into most to the right 2 stems (MRTS) of the entity) 
{
	static string stopwords[]={	"a", "about", "above", "above", "across", "after", "afterwards", "again", "against",
					"all", "almost", "alone", "along", "already", "also","although","always","am","among", 
					"amongst", "amoungst", "amount",  "an", "and", "another", "any","anyhow","anyone","anything","anyway", 
					"anywhere", "are", "around", "as",  "at", "back","be","became", "because","become","becomes", "becoming", 
					"been", "before", "beforehand", "behind", "being", "below", "beside", "besides", "between", "beyond", 
					"bill", "both", "bottom","but", "by", "call", "can", "cannot", "cant", "co", "con", "could", "couldnt", 
					"cry", "de", "describe", "detail", "do", "done", "down", "due", "during", "each", "eg", "eight", "either", 
					"eleven","else", "elsewhere", "empty", "enough", "etc", "even", "ever", "every", "everyone", "everything", 
					"everywhere", "except", "few", "fifteen", "fify", "fill", "find", "fire", "first", "five", "for", "former", 
					"formerly", "forty", "found", "four", "from", "front", "full", "further", "get", "give", "go", "had", "has", 
					"hasnt", "have", "he", "hence", "her", "here", "hereafter", "hereby", "herein", "hereupon", "hers", "herself",
					"him", "himself", "his", "how", "however", "hundred", "ie", "if", "in", "inc", "indeed", "interest", "into", 
					"is", "it", "its", "itself", "keep", "last", "latter", "latterly", "least", "less", "ltd", "made", "many", 
					"may", "me", "meanwhile", "might", "mill", "mine", "more", "moreover", "most", "mostly", "move", "much", 
					"must", "my", "myself", "name", "namely", "neither", "never", "nevertheless", "next", "nine", "no", "nobody", 
					"none", "noone", "nor", "not", "nothing", "now", "nowhere", "of", "off", "often", "on", "once", "one", "only", 
					"onto", "or", "other", "others", "otherwise", "our", "ours", "ourselves", "out", "over", "own","part", "per", 
					"perhaps", "please", "put", "rather", "re", "same", "see", "seem", "seemed", "seeming", "seems", "serious", 
					"several", "she", "should", "show", "side", "since", "sincere", "six", "sixty", "so", "some", "somehow", 
					"someone", "something", "sometime", "sometimes", "somewhere", "still", "such", "system", "take", "ten", 
					"than", "that", "the", "their", "them", "themselves", "then", "thence", "there", "thereafter", "thereby", 
					"therefore", "therein", "thereupon", "these", "they", "thickv", "thin", "third", "this", "those", "though", 
					"three", "through", "throughout", "thru", "thus", "to", "together", "too", "top", "toward", "towards", 
					"twelve", "twenty", "two", "un", "under", "until", "up", "upon", "us", "very", "via", "was", "we", "well", 
					"were", "what", "whatever", "when", "whence", "whenever", "where", "whereafter", "whereas", "whereby", 
					"wherein", "whereupon", "wherever", "whether", "which", "while", "whither", "who", "whoever", "whole", 
					"whom", "whose", "why", "will", "with", "within", "without", "would", "yet", "you", "your", "yours", 
					"yourself", "yourselves", "the"};
	static set<string> stopwordset(stopwords, stopwords+319);
	size_t startwordpos, endwordpos;

	static map<string, unsigned long long> StemId;
	static unsigned long long currentstemid=0;
	unsigned long long stemid;

	string stemseq;

	string result[MAXSTEMS]; // to store the two most to the right stems
	short int nb=0;
	wstring stem;
	stemming::english_stem<> StemEnglish;
	endwordpos=source.find_last_of("abcdedfghijklmnopqrstuvwxyz");
	startwordpos=source.find_last_of(" -'", endwordpos)+1;
	ostringstream sqlqry;

	while(startwordpos!=string::npos)
	{
		if(stopwordset.find(source.substr(startwordpos, endwordpos-startwordpos+1))==stopwordset.end()) // Current word is not a stopword
		{
			// transform it to a stem
			stem = string2wstring(source.substr(startwordpos,endwordpos-startwordpos+1));
			StemEnglish(stem);
			result[nb]=wstring2string(stem);
			nb++;
		
		}
		else	// ignore it
		{
		}
		if(startwordpos==0)
			break; // work is done
		endwordpos=source.find_last_of("abcdedfghijklmnopqrstuvwxyz", startwordpos-1); // previous word end;
		if(endwordpos==string::npos) // no previous word
			startwordpos=string::npos; // goodbye
		else
		{
			startwordpos=source.find_last_of(" -'",endwordpos)+1; // find start
			if(endwordpos==string::npos) // first word
				startwordpos=0; //first word
		}
	}
	if(nb==0)
		return;

	for(int i=0; i<nb; i++) // foreach i-stems category
	{
		stemseq="";
		for(int j=i; j>=0 ; j--)
		{
			if(j!=i)
				stemseq+=" ";
			stemseq+=result[j]; // concat stems
		}
		if(StemId.find(stemseq)==StemId.end())
		{// New Stems sequence
			StemId[stemseq]=++currentstemid;
			sqlqry<<"INSERT INTO StemSeqCat (scid, length, string) VALUES (";
			sqlqry<<"'"<<currentstemid<<"'"<<",'"<<i+1<<"' ,'"<<stemseq<<"');";
			try
    			{
      				work Insert(pgcon, "insert_StemSeqCat");
	 			disable_noticer d(pgcon);
     				Insert.exec(sqlqry.str());
    				Insert.commit();
    			}
    			catch (const sql_error &e)
    			{
      				cerr << "Couldn't insert into table: " << e.what() << endl
				<< "Query was: " << e.query() << endl;
				return;
    			}
			sqlqry.str("");
			stemid=currentstemid;
		}
		else 
			stemid=StemId[stemseq];
		// Add one occurence 
		// cerr << "StemId #"<<StemId[stemseq]<<" : "<< stemseq << endl;
		sqlqry<<"INSERT INTO StemSeqCatContain (scid, entid) VALUES (";
		sqlqry<<"'"<<stemid<<"'"<<",'"<<sourceid<<"');";
		try
    		{
      			work Insert(pgcon, "insert_StemSeqCatContain");
	 		disable_noticer d(pgcon);
     			Insert.exec(sqlqry.str());
    			Insert.commit();
    		}
    		catch (const sql_error &e)
    		{
      			cerr << "Couldn't insert into table: " << e.what() << endl
			<< "Query was: " << e.query() << endl;
			return;
    		}
		sqlqry.str("");							
	}
	return;
}

string GetDefinition(string context)
{
	int posinitial, i, j, searchstart, nbiter;
	char initial;
	string def;

	i=0;
	while(i<context.size())
	{
		context[i]=tolower(context[i]);
		i++;
	}
	posinitial=0;

	while(posinitial<context.size() && context[posinitial]!=')' && context[posinitial]!='<')
		posinitial++;
	initial=context[--posinitial];
	for(i=posinitial; context[i]!=' ' && context[i]!='-'&& context[i]!='\'';i--); // Go to the definition
	while(initial!='('&& initial != '>')
	{
		searchstart=i;
		nbiter=0;
		while(i>=0 && nbiter<=2 && context[i]!=initial) /* Find a word of the acronym in def */
		{
			/* Previous Word */
			if(context[i]==' '||context[i]=='-'||context[i]=='\'')
				i--;
			else 
				i-=2;
			while(i>=0 && context[i]!=' ' && context[i]!='-'&& context[i]!='\'')
				i--; 
			if(i<0 || context[i]==' '||context[i]=='-'|context[i]=='\'')
				i++;
			nbiter++;
		};
	if(Disaccent(context[i])!=initial)
		i=searchstart;
	else
		i--;	

	initial=context[--posinitial];
	}

	if(i<0)
		i=0;
	
	if(context[i]==' '||context[i]=='-'||context[i]=='\'')
		i++;
	if(initial=='('||initial=='<')
		posinitial++;
	while(context[posinitial]!=')' && context[posinitial]!='<')
		def.push_back(toupper(context[posinitial++]));
	def.push_back(':');
	while(i<context.size() && context[i]!='(')
		def.push_back(context[i++]);
	return def;
}


int main(int argc, char **argv)
{
	unsigned long long currentdocid=0;
	unsigned long long currententid=0;
	map <string, map <string, unsigned long long> > Entities;
	set <unsigned long long> SeenEntities;
	
	string currentdocname, currentline, pgconfig;

 	string basename, acrocontext, acro, acrodef, acrostem, acrocompact;
	ostringstream sqlqry;
    	boost::regex expression( "([ÀÁÂÃÄÅàáâãäåÒÓÔÕÖØòóôõöøÈÉÊËèéêëÌÍÎÏìíîïÙÚÛÜùúûüÿÑñÇçA-Za-z]+[- ']){2,10}\\((</?[A-Za-z]+>)?[A-Z][A-Z]+(</?[A-Za-z]+>)?(<sub>[a-z]+</sub>)?\\)" );
	string::const_iterator start, end; 
      	boost::match_results<std::string::const_iterator> what; 
   	boost::match_flag_type flags = boost::match_default; 

	ifstream fdoclist;
	ifstream fdocument;

	if(argc!=6)
	{
		cerr<<"Usage : "<<argv[0]<<" bdhost bduser bdpassword bdname doclist.txt"<<endl;
		exit(-1);
	}

  	pgconfig="host="+string(argv[1])+" port="+"5432"+" dbname="+string(argv[4])+" user="+string(argv[2])+" password="+string(argv[3]);
  	connection pgcon(pgconfig.c_str());
	
	try
	{
		DropTables(pgcon);
	} 
    	catch (const sql_error &e)
    	{
      		cerr << "SQL Error : " << e.what() << endl
	   	<< "Query was: " << e.query() << endl;
		return -1;
    	}
	try
    	{
		CreateTables(pgcon);
	}    	
	catch (const sql_error &e)
    	{
      		cerr << "SQL Error : " << e.what() << endl
	   	<< "Query was: " << e.query() << endl;
		return -1;
    	}
	//
	// Read the list of documents
	//
	fdoclist.open(argv[5]);
	fdoclist >> currentdocname;
	basename=currentdocname;
	basename=basename.substr(basename.find_last_of("/\\")+1);
	while(fdoclist)
	{
		fdocument.open(currentdocname.c_str());
		while(fdocument)
		{
			getline(fdocument, currentline);
			currentline=Transcode(currentline); // Deal with accents ("internationalization")
			start = currentline.begin(); 
		   	end = currentline.end(); 
		      	while(regex_search(start, end, what, expression, flags)) 
		  	{ 
				acrocontext= string(what[0].first, what[0].second) ;
				acrodef=GetDefinition(acrocontext); 
				if(acrodef.size()!=0)
				{
					acrocompact=acrodef.substr(0, acrodef.find_first_of(":"));
					acrodef=acrodef.substr(acrodef.find_first_of(":")+1, acrodef.size()-acrodef.find_first_of(":"));
					if((acrodef.size()!=0) && (acrocompact.size()!=0))
					{
						if(Entities.find(acrocompact)==Entities.end() || Entities[acrocompact].find(acrodef)==Entities[acrocompact].end() )
						{// New entity
							Entities[acrocompact][acrodef]=++currententid;
							acrodef=EscapeQuotes(acrodef);
							sqlqry<<"INSERT INTO Entities (entid,definition, initials) VALUES (";
							sqlqry<<"'"<<currententid<<"'"<<",'"<<acrodef<<"' ,'"<<acrocompact<<"');";
							try
    							{
      								work Insert(pgcon, "insert_Entities");
    			 					disable_noticer d(pgcon);
      								Insert.exec(sqlqry.str());
    								Insert.commit();
    							}
							catch (const sql_error &e)
    							{
      								cerr << "Couldn't insert into table: " << e.what() << endl
				 				<< "Query was: " << e.query() << endl
								<< "For article : "<< currentdocname <<endl;
								return -1;
    							}
							sqlqry.str("");
							// Insert Categories
							MRTS(pgcon, currententid, acrodef);
						}
						// Insert occurence
						if(SeenEntities.find(Entities[acrocompact][acrodef])==SeenEntities.end())
						{
							SeenEntities.insert(Entities[acrocompact][acrodef]);
							sqlqry<<"INSERT INTO Occurrences (entid,docid) VALUES (";
							sqlqry<<"'"<<Entities[acrocompact][acrodef]<<"'"<<",'"<<currentdocid<<"');";
							try
    							{
      								work Insert(pgcon, "insert_Occurrences");
    			 					disable_noticer d(pgcon);
      								Insert.exec(sqlqry.str());
    								Insert.commit();
    							}
    							catch (const sql_error &e)
    							{
      								cerr << "Couldn't insert into table: " << e.what() << endl
				 				<< "Query was: " << e.query() << endl;
								return -1;
    							}
							sqlqry.str("");
						}		
					}			
				}
      				start = what[0].second; 
      				// update flags: 
      				flags |= boost::match_prev_avail; 
      				flags |= boost::match_not_bob;
   			} 
		}
		fdocument.close();
		SeenEntities.clear();
		sqlqry<<"INSERT INTO Documents (docid,docname) VALUES (";
		sqlqry<<"'"<<currentdocid<<"'"<<",'"<<basename<<"');";
		try
    		{
      			work Insert(pgcon, "insert_Documents");
      			disable_noticer d(pgcon);
      			Insert.exec(sqlqry.str());
      			Insert.commit();
    		}
    		catch (const sql_error &e)
    		{
      			cerr << "Couldn't insert into table: " << e.what() << endl
				<< "Query was: " << e.query() << endl;
			return -1;
    		}
		sqlqry.str("");
		fdoclist >> currentdocname;
		basename=currentdocname;
		basename=basename.substr(basename.find_last_of("/\\")+1);
		if((currentdocid%10)==0)
			cerr<<"Processed "<<currentdocid<<" documents."<<(char) 13;
		currentdocid++;
	}
	fdoclist.close();

/*	sqlqry<<"CREATE VIEW MostGeneralClasses AS\
		SELECT f.nb, f.string, e.definition, e.initials\
		FROM\
			(SELECT count(e.definition) AS nb, s.scid, s.string\
			FROM stemseqcat s, stemseqcatcontain c, entities e\
			WHERE ((s.scid = c.scid) AND (c.entid = e.entid))\
			GROUP BY s.scid, s.string) f,\
			stemseqcatcontain c,\
			entities e\
		WHERE ((f.scid = c.scid) AND (c.entid = e.entid))\
		ORDER BY f.nb DESC, f.string, e.initials;";
	try
    	{
      		work View(pgcon, "create_view");
      		disable_noticer d(pgcon);
      		View.exec(sqlqry.str());
      		View.commit();
    	}
    	catch (const sql_error &e)
    	{
      		cerr << "Couldn't create view: " << e.what() << endl
			<< "Query was: " << e.query() << endl;
		return -1;
    	}*/
	return 0;
}
