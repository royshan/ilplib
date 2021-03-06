/*
 * =====================================================================================
 *
 *       Filename:  fill_naive_bayes.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2013年05月24日 14时09分51秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Kevin Hu (), kevin.hu@b5m.com
 *        Company:  B5M.com
 *
 * =====================================================================================
 */
#include <string>
#include<iostream>
#include<fstream>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

#include "knlp/tokenize.h"
#include "knlp/normalize.h"
#include "am/hashtable/khash_table.hpp"
#include "am/util/line_reader.h"
#include "net/seda/queue.hpp"

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

using namespace std;
using namespace ilplib::knlp;
using namespace izenelib::am;
using namespace izenelib::am::util;
using namespace izenelib;

/* 
 * mu(t) = N(t)/N
 * X(c, t) = N(c, t)/N(c)
 * Delta(c, t) = sqrt((pow((N(c)-N(c,t))/N(c), 2)*(N(c)-N(c,t))+N(c,t)*pow(N(c,t)/N(c),2))/(N(c)-1))
 *
 * (X(c,t)- mu(t))/Delta(c, t)*sqrt(N(c)-1)
 *
 * */

ilplib::knlp::Tokenize* tkn;
KStringHashTable<string,uint32_t>*	Nt;
KStringHashTable<string,uint32_t>* Nc;
KStringHashTable<string,uint32_t>* Nct;
std::set<string> cates;
std::vector<string> tks;
uint32_t N = 0;
double MAX_IDF;
typedef KStringHashTable<string,uint32_t> freq_t;

std::pair<double,double>
t_test(const string& c, const string& t)
{
	double nt = 0, nct = 0, nc = 0;
	{
		uint32_t* f = Nt->find(t);
		if (f) nt = *f;
	}
	{
		string ct = t;ct+='\t';ct+=c;
		uint32_t* f = Nct->find(ct);
		if (f) nct = *f;
	}
	{
		uint32_t* f = Nc->find(c);
		IASSERT(f);
		if (f) nc = *f;
	}
	//std::cout<<c<<":"<<t<<":"<<nt<<":"<<nct<<":"<<nc<<":"<<nct/nc<<":"<<nt/N<<std::endl;

	return make_pair((nct/nc - nt/N)*sqrt(nc-1)/sqrt(((nc-nct)*pow((nc-nct)/nc,2)+nct*nct*nct/nc/nc)/(nc-1)), 
				pow(std::min((nc-nct+0.5)/(nct+0.5),MAX_IDF)/MAX_IDF, 2));
}

std::pair<double,double>
chi_square_test(const string& ca, const string& t)
{
	double nt = 0, nct = 0, nc = 0;
	{
		uint32_t* f = Nt->find(t);
		if (f) nt = *f;
	}
	{
		string ct = t;ct+='\t';ct+=ca;
		uint32_t* f = Nct->find(ct);
		if (f) nct = *f;
	}
	{
		uint32_t* f = Nc->find(ca);
		IASSERT(f);
		if (f) nc = *f;
	}
	//std::cout<<c<<":"<<t<<":"<<nt<<":"<<nct<<":"<<nc<<":"<<nct/nc<<":"<<nt/N<<std::endl;
	double a = nct, b = nc-nct, c = nt-nct, d=N-nc-nt+nct;
	a /= 100, b/=100, c/=100, d/=100;
	//std::cout<<a<<":"<<b<<":"<<c<<":"<<d<<std::endl;

	return make_pair(pow(a*d-b*c, 2)*N/(a+b)/(c+d)/(a+c)/(b+d), 
	  pow(std::min((nc-nct+0.5)/(nct+0.5), MAX_IDF)/MAX_IDF, 2));
}

double nct_given_nt(const string& ca, const string& t)
{
	double nt = 0, nct = 0;//, nc = 0;
	{
		uint32_t* f = Nt->find(t);
		if (f) nt = *f;
	}
	{
		string ct = t;ct+='\t';ct+=ca;
		uint32_t* f = Nct->find(ct);
		if (f) nct = *f;
	}
	/*{
		uint32_t* f = Nc->find(ca);
		IASSERT(f);
		if (f) nc = *f;
	}*/
	//std::cout<<c<<":"<<t<<":"<<nt<<":"<<nct<<":"<<nc<<":"<<nct/nc<<":"<<nt/N<<std::endl;

	//return (nct+1)/(nt+10000);
	return nct/nt;
}

void calculate_stage(EventQueue<std::pair<string*,string*> >* out)
{
	while(true)
	{
		uint64_t e = -1;
		std::pair<string*,string*> p(NULL,NULL);
		out->pop(p, e);
		string* t = p.first;
		string* c = p.second;
		if (t == NULL && c == NULL)
		  break;
		
        {
            //add Nc
            if (cates.find(*c) == cates.end())
            {
                cates.insert(*c);
                Nc->insert(*c, 1);
            }
            else{
                uint32_t* f = Nc->find(*c);
                IASSERT(f);
                (*f)++;
            }
        }

		{//Nt
			uint32_t* f = Nt->find(*t);
			if (!f)
			{
				Nt->insert(*t,1);
				tks.push_back(*t);
			}
			else (*f)++;
		}{//Nct
			(*t) += '\t';(*t) += (*c);
			uint32_t* f = Nct->find(*t);
			if (!f)
				Nct->insert(*t,1);
			else (*f)++;
		}

		delete t, delete c;
	}
}

void tokenize_stage(EventQueue<std::pair<string*,string*> >* in, 
			EventQueue<std::pair<string*,string*> >* out )
{
	while(true)
	{	
		uint64_t e = -1;
		std::pair<string*,string*> p(NULL, NULL);
		in->pop(p, e);
		string* t = p.first;
		string* c = p.second;
		if (t == NULL || c == NULL)
		  break;
		ilplib::knlp::Normalize::normalize(*t);
		std::vector<KString> v = tkn->fmm(KString(*t));
		std::set<string> s;
		for ( uint32_t i=0; i<v.size(); ++i)
		  if (v[i].length() > 0 && tkn->is_in(v[i]))//&& KString::is_chinese(v[i][0]))
			  s.insert(v[i].get_bytes("utf-8"));
		
		for ( std::set<string>::iterator it=s.begin(); it!=s.end(); ++it)
			out->push(make_pair(new string(it->c_str()), new string(c->c_str())), e);
//		out->push(make_pair<std::string*, std::string*>(NULL, new string(c->c_str())), e);
		delete t, delete c;
	}
}

double standard_deviation(const std::vector<std::pair<std::pair<double,double>, string> >& v)
{
    double sd = 0, aver=0;
    for (uint32_t i=0;i<v.size();++i)
        aver += v[i].first.first;
    aver /= v.size();
    for (uint32_t i=0;i<v.size();++i)
        sd += (v[i].first.first-aver)*(v[i].first.first-aver);
    return sqrt(sd/v.size());
}

void output_ttest()
{
	for ( uint32_t i=0; i<tks.size(); ++i)
	{
		std::vector<std::pair<std::pair<double,double>, string> > v;
		v.reserve(cates.size());
		for ( std::set<string>::iterator it=cates.begin(); it!=cates.end(); ++it)
		{
			std::pair<double,double> p = t_test(*it, tks[i]);
			v.push_back(make_pair(p, *it));
		}
        //std::cout<<tks[i]<<"\t"<<standard_deviation(v)<<std::endl;
		std::vector<std::pair<std::pair<double,double>, string> >::iterator it = max_element(v.begin(), v.end());
		std::cout<<tks[i]<<"\t"<<it->first.first*it->first.second<<std::endl;
	}
}

void output_chisquare()
{
	for ( uint32_t i=0; i<tks.size(); ++i)
	{
		std::vector<std::pair<std::pair<double,double>, string> > v;
		v.reserve(cates.size());
		for ( std::set<string>::iterator it=cates.begin(); it!=cates.end(); ++it)
		{
			std::pair<double,double> p = chi_square_test(*it, tks[i]);
			v.push_back(make_pair(p, *it));
		}
        //std::cout<<tks[i]<<"\t"<<standard_deviation(v)<<std::endl;
		std::vector<std::pair<std::pair<double,double>, string> >::iterator it = max_element(v.begin(), v.end());
		std::cout<<tks[i]<<"\t"<<it->first.first*it->first.second<<std::endl;
	}
}

void output_standard_dev()
{
	for ( uint32_t i=0; i<tks.size(); ++i)
	{
		std::vector<std::pair<double, string> > v;
		v.reserve(cates.size());
		double aver = 0;
		for ( std::set<string>::iterator it=cates.begin(); it!=cates.end(); ++it)
		{
			double p = nct_given_nt(*it, tks[i]);
			aver += p;
			v.push_back(make_pair(p, *it));
		}
		aver /= cates.size();
		double sd = 0;
		for (uint32_t j=0;j<v.size();++j)
		    sd += (v[j].first-aver)*(v[j].first-aver);
		sd = sqrt(sd/v.size());
		std::cout<<tks[i]<<"\t"<<sd<<std::endl;
	}
}

boost::mutex io_mutex;
void print(const std::string& tk, double s)
{
	 boost::mutex::scoped_lock lock(io_mutex);
	 std::cout<<tk<<"\t"<<s<<std::endl;
}

void ouput_entropy(std::vector<string>::iterator begin,
			std::vector<string>::iterator end)
{
    for (std::vector<string>::iterator t=begin;t!=end;++t)
	{
		double H = 0;
		for ( std::set<string>::iterator it=cates.begin(); it!=cates.end(); ++it)
		{
			double p = nct_given_nt(*it, *t);
			if (p != 0.)
			    H += p*log(p);
		}
		print(*t, H);
	}
}

int main(int argc,char * argv[])
{
	if (argc < 4)
	{
		std::cout<<argv[0]<<" [tokenize dict] [MAX_IDF] [corpus 1] [corpus 2] ....\n\t[format]: doc\\tcategory\n";
		return 0;
	}

	string dictnm = argv[1];
	MAX_IDF = atof(argv[2]);
	std::vector<std::string> cps;

	for ( int32_t i=3; i<argc; ++i)
	  cps.push_back(argv[i]);

	tkn = new ilplib::knlp::Tokenize(dictnm);
	Nt = new freq_t(tkn->size()*3, tkn->size()+3);
	Nc = new freq_t(300, 100);
	Nct = new freq_t(tkn->size()*3*100, tkn->size()*100+3);
	tks.reserve(tkn->size());

	EventQueue<std::pair<string*,string*> > in, out;
	uint32_t cpu_num = 11;
	std::vector<boost::thread*> token_ths;
	for ( uint32_t i=0; i<cpu_num; ++i)
	  token_ths.push_back(new boost::thread(&tokenize_stage, &in, &out));
	boost::thread cal_th(&calculate_stage, &out);

    N = 0;
	for ( uint32_t i=0; i<cps.size(); ++i)
	{
		LineReader lr(cps[i]);
		char* line = NULL;
		while((line=lr.line(line))!=NULL)
		{
			if (strlen(line) == 0)continue;
			char* t = strchr(line, '\t');
			if (!t)continue;
			t++;
			if (strlen(t) == 0)continue;

			in.push(make_pair(new std::string(line, t-line-1), new string(t)), -1);
			N++;
		}
	}

	for ( uint32_t i=0; i<cpu_num; ++i)
	  in.push(make_pair<string*,string*>(NULL, NULL), -1);
	for ( uint32_t i=0; i<cpu_num; ++i)
	  token_ths[i]->join(),delete token_ths[i];
	out.push(make_pair<string*,string*>(NULL, NULL), -1);
	cal_th.join();

    //output_standard_dev();
    //output_ttest();
    //output_chisquare();
	uint32_t gap = tks.size()/cpu_num;
	token_ths.clear();
	for ( uint32_t i=0; i*gap<tks.size(); i++)
		token_ths.push_back(new boost::thread(&ouput_entropy, tks.begin()+i*gap, 
						((i+1)*gap<tks.size() ?(tks.begin()+(i+1)*gap) : tks.end())));
	for (uint32_t i=0;i<token_ths.size();++i)
	  token_ths[i]->join(),delete token_ths[i];

	return 0;
}
