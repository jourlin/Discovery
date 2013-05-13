/* LBDCoreSimpleStemCat.cpp : Get stemseqcat occurences from SQL server and produces a ranked list of hypothesis

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
  EntityExtractor.cpp : Obtient les occurences de stemseqcat du serveur SQL et produit une liste ordonnée d'hypothèses

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
#include <sstream>
#include <string>
#include <pqxx/connection>
#include <pqxx/transaction>
#include <pqxx/pqxx>

using namespace std;
using namespace pqxx;
using namespace PGSTD;

class set 
{
public :
	vector<unsigned long long> element; // each unsigned long long element contains 64 existence flag
public : 
	set(unsigned long long fixedelements) // Constructs an Empty set
	{
		element.resize(fixedelements/64+2,0);
	};
	~set()
	{
	 	element.clear();
	}
	
	set(const set& source)
	{
		for(int i=0;i<source.element.size(); i++) 
			element.push_back(source.element[i]);

	}
	void add(unsigned long long int i)
	{
		element[i/64]=element[i/64] | (0x8000000000000000 >> (i%64));
	};
	void del(unsigned long long int i)
	{
		element[i/64]=element[i/64] | (0x8000000000000000 >> (i%64));
	};
	bool test(unsigned long long int i)
	{
		return (
					element[i/64] 
					& 
					(
						0x8000000000000000 >> (i%64)
					)
			)
			!=0;
	};
	void show()
	{
		unsigned long long int i;
		cout <<"{";
		for(i=0;test(i)==false && (i<element.size()*64-1); i++); 
		if(test(i))
			cout << i;
		for(i++; i<element.size()*64; i++)
			if(test(i))
				cout << ", " << i ;
		cout << "}";
	};
	bool isempty()
	{
		int i=0;
		while(i<element.size() && element[i]==0)
			i++;
		return i==element.size();
	};
	unsigned long long count()
	{
		unsigned long long int i, j, sum=0;
		for(i=0; i<element.size(); i++)
			if(element[i]!=0)
				for(j=0;j<64;j++)
					if(test(i*64+j))
						sum++;
		return sum;
	};	
	set& operator=(const set &source);	
		
};

set& set::operator=(const set &source)
{
    if(this->element.size()!=source.element.size())
		{
		 	cout << "Error in assign operator : the two sets are incompatible"<<endl;
			cout << "left element size :" << this->element.size()<<endl;
			cout << "right element size :" << source.element.size()<<endl;
			throw("Error in set operator =");
		};
    // do the copy
    for(int i=0; i<source.element.size(); i++)
	element[i]=source.element[i];
    // return the existing object
    return *this;
}

bool operator==(const set& a, const set& b) // a and b are equal
{
	unsigned long long i=0;
	if(a.element.size()!=b.element.size())
		{
		 	cout << "Error in equality test : the two sets are not comparable"<<endl;
			throw("Error in set operator ==");
		};
	while(i<a.element.size() && a.element[i]==b.element[i])
		i++;
	return i==a.element.size();	
}
bool operator<=(const set& a, const set& b) // a is included in b
{
	unsigned long long i=0;
	if(a.element.size()!=b.element.size())
		{
		 	cout << "Error in inclusion test : the two sets are not comparable"<<endl;
			throw("Error in set operator <=");
		};
	while(i<a.element.size() && a.element[i]==(a.element[i]&b.element[i]))
		i++;
	return i==a.element.size();	
}
bool operator>=(const set& a, const set& b) // a embodies b
{
	unsigned long long i=0;
	if(a.element.size()!=b.element.size())
		{
		 	cout << "Error in embody test : the two sets are not comparable"<<endl;
			throw("Error in set operator >=");
		};
	while(i<a.element.size() && b.element[i]==(b.element[i]&a.element[i]))
		i++;
	return i==a.element.size();	
}
bool operator!=(const set& a, const set& b) // a and b do not intersect
{
	unsigned long long i=0;
	if(a.element.size()!=b.element.size())
		{
		 	cout << "Error in intersection test : the two sets are incompatible"<<endl;
			throw("Error in set operator !=");
		};
	while(i<a.element.size() && 0==(b.element[i]&a.element[i]))
		i++;
	return i==a.element.size();	
}
set operator*(const set& a, const set& b) // returns the intersection of a et b
{
	unsigned long long i=0;
	if(a.element.size()!=b.element.size())
		{
		 	cout << "Error in intersection : the two sets are incompatible"<<endl;
			cout << "Left set domain cardinality : "<<a.element.size()<<endl;
			cout << "Right set domain cardinality : "<<b.element.size()<<endl;
			throw("Error in set operator *");
		};
	set c(a.element.size());
	c.element.resize(a.element.size(),0);
	while(i<a.element.size())
		{
		  c.element[i]=b.element[i]&a.element[i];
		  i++;
		}
	return c;
}
set operator-(const set& a, const set& b) // returns a minus b
{
	unsigned long long i=0;
	if(a.element.size()!=b.element.size())
		{
		 	cout << "Error in operator minus : the two sets are incompatible"<<endl;
			throw("Error in set operator -");
		};
	set c(a.element.size());
	c.element.resize(a.element.size(),0);
	while(i<a.element.size())
		{
		  c.element[i]=a.element[i]-(b.element[i]&a.element[i]);
		i++;
		}
	return c;
}

bool EmptyIntersection(unsigned long long a, unsigned long long b, map<unsigned long long, set*>& DocSet)
{
	static map<unsigned long long, map<unsigned long long, bool> > BoolStore;
	if((BoolStore.find(a)!=BoolStore.end()) && (BoolStore[a].find(b)!=BoolStore[a].end()))
		return BoolStore[a][b];
	if((BoolStore.find(b)!=BoolStore.end()) && (BoolStore[b].find(a)!=BoolStore[b].end()))
		return BoolStore[b][a];
	BoolStore[a][b]= *(DocSet[a]) != *(DocSet[b]);
	return BoolStore[a][b];
}

void DropTables(connection& pgcon)
{
	string TableNames[1]={"ListOfHypothesis"};
	for(int i=0; i<1; i++)
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
	string TableNames[1]={"ListOfHypothesis"};
	string TableQry[1];
	TableQry[0]="CREATE TABLE ListOfHypothesis (classa bigint NOT NULL, classb bigint NOT NULL, classc bigint NOT NULL, nba bigint NOT NULL, nbb bigint NOT NULL, nbc bigint NOT NULL)";
	for(int i=0; i<1; i++)
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

void CreateView(connection& pgcon)
{
	try
	{
	      	work CView(pgcon, "create_view");
	      	disable_noticer d(pgcon);
	      	CView.exec("CREATE VIEW orderedstemseqcat AS SELECT s.scid, s.length, s.string, count(o.docid) as nbdocs FROM stemseqcat as s, stemseqcatcontain as c, occurrences as o WHERE c.scid=s.scid AND o.entid=c.entid GROUP by s.scid, s.length, s.string ORDER by nbdocs");
	      	CView.commit();
	}
	//failed to create table
  	catch (const sql_error &e)
	{
		throw(e);
   	}
}

void DropView(connection& pgcon)
{
	try
	{
	      	work DView(pgcon, "drop_view");
	      	disable_noticer d(pgcon);
	      	DView.exec("DROP VIEW orderedstemseqcat");
	      	DView.commit();
	}
	//failed to create table
  	catch (const sql_error &e)
	{
		throw(e);
   	}
}

int main(int argc, char **argv)
{
	string pgconfig;
	unsigned long long docid;
	unsigned long long stemseqcat;
	unsigned long long nbdocuments;
	unsigned long long nbstemseq;
	string sqlqry;
	ostringstream sqlins;

	int offset=0;
	unsigned long long NROWS=100ULL;
	string SSCL="2";

	while(argc>=1+offset && argv[1+offset][0]=='-')
	{	
		char var=argv[1+offset][1];
		if(var!='l' && var !='n')
		{
			cerr<<"Usage : "<<argv[0]<<" [-l<stemseqlength>][-n<numberofclasses>] bdhost bduser bdpassword bdname"<<endl;
			return -1;
		}
		
		string valstr=string(argv[1+offset]);
		valstr=valstr.substr(2,valstr.length()-2);
		istringstream iss(valstr);
		unsigned long long val;
		iss>>val;
		if(var=='n')
			NROWS=val;
		else
			SSCL=valstr;
		cerr << "Got option "<<var<<" = "<<val<<endl;
		offset++;
	}

	if(argc!=5+offset)
	{
		cerr<<"Usage : "<<argv[0]<<" [-l<stemseqlength>][-n<numberofclasses>] bdhost bduser bdpassword bdname"<<endl;
		return -1;
	}

  	pgconfig="host="+string(argv[1+offset])+" port="+"5432"+" dbname="+string(argv[4+offset])+" user="+string(argv[2+offset])+" password="+string(argv[3+offset]);
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
	
	// Get the number of distinct SSCL-stem classes 

	sqlqry="SELECT max(s.scid) FROM stemseqcatcontain as c, stemseqcat as s, occurrences as o WHERE s.length=";
	sqlqry=sqlqry+SSCL+" AND c.scid=s.scid AND o.entid=c.entid";

	try
  	{
    		work Select(pgcon, "countscid");
		result R( Select.exec(sqlqry) );
    		if (R.empty()) throw logic_error("No occurrences found");
    		// Process each occurence
    		result::const_iterator c = R.begin();
    		nbstemseq=c[0].as(nbstemseq);		
		cerr <<"Found "<<nbstemseq<<" stem sequence classes."<<endl;
		Select.commit();
  	}
  	catch (const sql_error &e)
  	{
    		cerr << "SQL error: " << e.what() << endl
         	<< "Query was: " << e.query() << endl;
    		return 1;
  	}
  	catch (const exception &e)
  	{
    		cerr << "Exception: " << e.what() << endl;
    		return 2;
  	}
	// Get the number of distinct documents in which they occur
	sqlqry="SELECT max(o.docid) FROM stemseqcatcontain as c, stemseqcat as s, occurrences as o WHERE s.length=";
	sqlqry=sqlqry+SSCL+" AND c.scid=s.scid AND o.entid=c.entid";
	try
  	{
    		work Select(pgcon, "countdocid");
		result R( Select.exec(sqlqry) );
    		if (R.empty()) throw logic_error("No occurrences found");
    		// Process each occurence
    		result::const_iterator c = R.begin();
    		nbdocuments=c[0].as(nbdocuments);		
		cerr <<"Found "<<nbdocuments<<" documents."<<endl;
		Select.commit();
  	}
  	catch (const sql_error &e)
  	{
    		cerr << "SQL error: " << e.what() << endl
         	<< "Query was: " << e.query() << endl;
    		return 1;
  	}
  	catch (const exception &e)
  	{
    		cerr << "Exception: " << e.what() << endl;
    		return 2;
  	}

	map<unsigned long long, set*> DocSet;
	
	// StemSeqClass occurrences
	try
	{
		CreateView(pgcon);
	}
  	catch (const sql_error &e)
  	{
    		cerr << "SQL error: " << e.what() << endl
         	<< "Query was: " << e.query() << endl;
    		return 1;
  	}
  	catch (const exception &e)
  	{
    		cerr << "Exception: " << e.what() << endl;
    		return 2;
  	}
	sqlqry="SELECT DISTINCT s.scid, o.docid, s.nbdocs FROM orderedstemseqcat as s, stemseqcatcontain as c, occurrences as o WHERE s.length=";
	sqlqry=sqlqry+SSCL+" AND c.scid=s.scid AND o.entid=c.entid ORDER by s.nbdocs DESC";
	try
  	{
    		work Select(pgcon, "stemseqclasses");
		result R( Select.exec(sqlqry) );
    		if (R.empty()) throw logic_error("No occurrences found");

    		// Process each occurence
    		for (result::const_iterator c = R.begin(); c != R.end(); ++c)
    		{
			stemseqcat=c[0].as(stemseqcat);
			docid=c[1].as(docid);			
			//cout << stemseqcat << '\t' << docid << endl;
			if(DocSet.find(stemseqcat)==DocSet.end())
				DocSet[stemseqcat]=new set(nbdocuments);
			DocSet[stemseqcat]->add(docid);
    		}
		Select.commit();
  	}
  	catch (const sql_error &e)
  	{
    		cerr << "SQL error: " << e.what() << endl
         	<< "Query was: " << e.query() << endl;
    		return 1;
  	}
  	catch (const exception &e)
  	{
    		cerr << "Exception: " << e.what() << endl;
    		return 2;
  	}
	DropView(pgcon);
	// Find Unpublished
	map<unsigned long long, set*>::iterator i, j;
	map<unsigned long long, set*>::reverse_iterator k;
	unsigned long long nbi, nbj, nbk;
	nbi=NROWS;
	for(i=DocSet.begin(); (nbi>0) && (i!=DocSet.end());i++)
	{
		nbi--;
		nbj=NROWS;
		for(j=i; (nbj>0) && (j!=DocSet.end()); j++)
		{
			//if((j->first%10)==0)
				cerr<<"Comparing entity "<<i->first<<" with entity "<<j->first<<"      "<<(char) 13;

			if(EmptyIntersection(i->first, j->first, DocSet))		//  unpublished relation between i & j
			{
				nbj--;
				nbk=NROWS;
				for(k=DocSet.rbegin(); (nbk>0) && (k!=DocSet.rend()); k++)
					if((!EmptyIntersection(i->first, k->first, DocSet))&&(!EmptyIntersection(k->first, j->first, DocSet))) // Find a link 
						{
							nbk--;
							sqlins.str("");								
							sqlins<<"INSERT INTO ListOfHypothesis (classa, classb, classc, nba, nbb, nbc) VALUES (";
							sqlins<<"'"<<i->first<<"'"<<",'"<<k->first<<"' ,'"<<j->first<<"','"<<i->second->count()<<"','"<<k->second->count()<<"','"<<j->second->count()<<"');";
							try
    							{
      								work Insert(pgcon, "insert_Entities");
    								disable_noticer d(pgcon);
      								Insert.exec(sqlins.str());
    								Insert.commit();
    							}
							catch (const sql_error &e)
    							{
      								cerr << "Couldn't insert into table: " << e.what() << endl
			 					<< "Query was: " << e.query() << endl;
								return -1;
    							}
							sqlins.str("");								
							sqlins<<"INSERT INTO ListOfHypothesis (classa, classb, classc, nba, nbb, nbc) VALUES (";
							sqlins<<"'"<<j->first<<"'"<<",'"<<k->first<<"' ,'"<<i->first<<"','"<<j->second->count()<<"','"<<k->second->count()<<"','"<<i->second->count()<<"');";
							try
    							{
      								work Insert(pgcon, "insert_Entities");
    		 						disable_noticer d(pgcon);
      								Insert.exec(sqlins.str());
    								Insert.commit();
    							}
							catch (const sql_error &e)
    							{
      								cerr << "Couldn't insert into table: " << e.what() << endl
			 					<< "Query was: " << e.query() << endl;
								return -1;
    							}

						}
				}	
			}
	}
	return 0;
// Get nbdocs for each 1-stem sequence : 
// SELECT s.scid, s.string, count(o.docid) as nbdocs FROM stemseqcatcontain as c, stemseqcat as s, occurrences as o WHERE s.length=1 AND c.scid=s.scid AND o.entid=c.entid GROUP BY s.scid, s.string ORDER BY nbdocs DESC

}
