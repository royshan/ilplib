/*
 * =====================================================================================
 *
 *       Filename:  tokenize.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  01/12/2013 06:05:21 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Kevin Hu (), kevin.hu@b5m.com
 *        Company:  iZeneSoft.com
 *
 * =====================================================================================
 */
#ifndef _ILPLIB_NLP_FMM_H_
#define _ILPLIB_NLP_FMM_H_

#include <string>
#include <vector>
#include <list>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <istream>
#include <ostream>
#include <cctype>
#include <math.h>

#include "util/string/kstring.hpp"
#include "trd2simp.h"
#include "am/util/line_reader.h"
#include "datrie.h"
//#include "knlp/william_trie.h"

namespace ilplib
{
namespace knlp
{

class Fmm
{
    DATrie trie_;
    //WilliamTrie trie_;
    std::vector<uint8_t> delimiter_;

    void chunk_(const KString& line, 
      std::vector<KString>& r, bool deli=true)
    {
        int32_t la = 0;
        for ( int32_t i=0; i<(int32_t)line.length(); ++i)
            if (KString::is_english(line[i])||KString::is_numeric(line[i]))
            {
                if (la < i)
                {
                    r.push_back(line.substr(la, i-la));
                    la = i;
                }
                while(i<(int32_t)line.length() && (is_alphanum_(line[i]) || (!deli && line[i] == ' ')))
                    i++;
                while((int32_t)i>0 && i < (int32_t)line.length() && (!deli && line[i] == ' '))
                    i--;
                if (i -la <= 1)
                {
                    --i;
                    continue;
                }
                r.push_back(line.substr(la, i-la));
                la = i;
                i--;
            }
            else if (deli && delimiter_[line[i]])
            {
                //std::cout<<line.substr(la, i-la+1)<<"OOOOOO\n";
                if (i > la)r.push_back(line.substr(la, i-la));
                r.push_back(line.substr(i, 1));
                la = i+1;
            }

        if (la < (int32_t)line.length())
            r.push_back(line.substr(la));
    }

    bool is_digit_(int c)const
    {
        return c >= '0' && c<='9';
        return is_digit(c);
    }

    bool is_alphanum_(int c)
    {
        if (KString::is_english(c) || is_digit_(c)
                || c==' ' || c=='\'' || c=='-' || c=='.'
                || c == '$' || c=='%')
            return true;
        return false;
    }

    bool is_alphanum_(const KString& str)
    {
        for ( uint32_t i=0; i<str.length(); ++i)
            if(!is_alphanum_(str[i]))
                return false;
        return true;
    }

public:

    Fmm(const std::string& dict_nm)
        :trie_(dict_nm)
    {
        KString ustr("~！@#￥……&*（）—+【】{}：“”；‘’、|，。《》？ ^()-_=[]\\|;':\"<>?/　");
        delimiter_.resize((int)((uint16_t)-1), 0);
        for ( uint32_t i=0; i<ustr.length(); ++i)
            delimiter_[ustr[i]] = 1;
    }

    ~Fmm()
    {
    }

    static bool ischinese(const KString& str)
    {
        for (uint32_t i=0; i<str.length(); i++)
            if (! KString::is_chinese(str[i]))
                return false;
        return true;
    }

    static bool is_digit(int c)
    {
        return (std::isdigit(c)||c=='.'||c=='%'||c=='$'||c==','||c=='-');
    }

    bool is_in(const KString& kstr)
    {
        return trie_.check_term(kstr);
    }

    double score(const KString& kstr)
    {
        return trie_.score(kstr);
    }

    void fmm(const KString& line, std::vector<std::pair<KString,double> >& r, bool smart=true, bool bigterm=true)//forward maximize match
    {
        r.clear();
        if (line.length() == 0)return;

        std::vector<KString> chunks;
        chunk_(line, chunks, (!bigterm));
        for ( uint32_t i=0; i<chunks.size(); ++i)
            if(chunks[i].length()>0)
            {
                if (smart && (is_alphanum_(chunks[i])
                              || (chunks[i].length() < 3 && ischinese(chunks[i]))
                              || (chunks[i].length() == 3 && ischinese(chunks[i]))))
                {
                    chunks[i].trim_head_tail();
                    //std::cout<<chunks[i]<<std::endl;
                    if (chunks.size() > 1 && chunks[i].length() < 18)
                        r.push_back(make_pair(chunks[i], trie_.score(chunks[i])));
                    else
                    {
                        std::vector<KString> sp = chunks[i].split(' ');
                        for (uint32_t t=0; t<sp.size(); t++)
                            r.push_back(make_pair(sp[t], trie_.score(sp[t])));
                    }
                    continue;
                }
                std::vector<std::pair<KString,double> > v;
                trie_.token(chunks[i], v);
                r.insert(r.end(), v.begin(), v.end());
            }
        for (uint32_t i=0; i<r.size(); ++i)
        {
            r[i].first.trim_head_tail();
            if (r[i].first.length() > 1)
                continue;
            if (r[i].first.length() >0 && r[i].first[0] != ' ')
                continue;
            r.erase(r.begin()+i);
            --i;
        }
    }

    std::vector<std::pair<KString,double> >
    subtokens(std::vector<std::pair<KString,double> > tks)
    {
        std::vector<std::pair<KString,double> > r;
        for (uint32_t i=0; i<tks.size(); ++i)
        {
            //std::cout<<tks[i].first<<" xxxxx\n";
            for(uint32_t j=0; j<tks[i].first.length(); j++)
                if (tks[i].first[j] == ' ')
                    tks[i].first[j] = ',';
            std::vector<KString> t;
            chunk_(tks[i].first, t);
            if (t.size() > 1)
            {
                for (uint32_t j=0; j<t.size(); ++j)
                    if (t[j].length() == 1 && t[j][0] == ',')continue;
                    else r.push_back(make_pair(t[j], trie_.score(t[j])));
                continue;
            }
            if (tks[i].first.length() <4
                    ||(tks[i].first.length() > 0 && (KString::is_english(tks[i].first[0]) || KString::is_numeric(tks[i].first[0]))))
            {
                //std::cout<<tks[i].first<<" sssss\n";
                r.push_back(tks[i]);
                continue;
            }
            std::vector<std::pair<KString,double> > vv;
            trie_.sub_token(tks[i].first, vv);
            r.insert(r.end(), vv.begin(), vv.end());
        }
        return r;
    }

    static void bigram_with_space(std::vector<std::pair<KString,double> >& r)
    {
        for (uint32_t i=0; i<r.size()-1; ++i)
        {
            r[i].first += ' ';
            r[i].first += r[i+1].first;
            r[i].second = (r[i].second+r[i+1].second)/2.;
        }
        r.erase(r.begin()+r.size()-1);
    }

    static void bigram(std::vector<std::pair<KString,double> >& r)
    {
        for (uint32_t i=0; i<r.size()-1; ++i)
        {
            r[i].first += r[i+1].first;
            r[i].second = (r[i].second+r[i+1].second)/2.;
        }
        r.erase(r.begin()+r.size()-1);
    }

    static void gauss_smooth(std::vector<double>& r, double alpha=0.4)
    {
        for (uint32_t i=0; i<r.size(); ++i)
        {
            if (i >= 1 && i<r.size()-1)
                r[i] = r[i]*alpha + (r[i+1]+r[i-1])*(1.-alpha)/2;
            else if(i == 0 && i+1 < r.size())
                r[i] = r[i]*alpha + r[i+1]*(1.-alpha);
            else if (i == r.size() -1 && i>=1)
                r[i] = r[i]*alpha + r[i-1]*(1.-alpha);
        }
    }

    static void gauss_smooth(std::vector<std::pair<KString,double> >& r, double alpha=0.4)
    {
        for (uint32_t i=0; i<r.size(); ++i)
        {
            if (i >= 1 && i<r.size()-1)
                r[i].second = r[i].second*alpha + (r[i+1].second+r[i-1].second)*(1.-alpha)/2;
            else if(i == 0 && i+1 < r.size())
                r[i].second = r[i].second*alpha + r[i+1].second*(1.-alpha);
            else if (i == r.size() -1 && i>=1)
                r[i].second = r[i].second*alpha + r[i-1].second*(1.-alpha);
        }
    }

    std::vector<KString> fmm(const KString& line, bool smart=true, bool bigterm=true)//forward maximize match
    {
        std::vector<std::pair<KString,double> > v;
        fmm(line, v, smart, true);
        std::vector<KString> r;
        for ( uint32_t i=0; i<v.size(); ++i)
            r.push_back(v[i].first);
        return r;
    }

    uint32_t size()const
    {
        return trie_.size();
    }

    double min()const
    {
        return trie_.min();
    }

    std::size_t termid(const KString& t)
    {
        return trie_.find_word(t);
    }

    double score(const size_t ind)
    {
        return trie_.score(ind);
    }

    KString id2term(uint32_t id)
    {
        return trie_.id2word(id);
    }
};

}
}//namespace

#endif

