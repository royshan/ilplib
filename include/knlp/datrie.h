#ifndef _ILPLIB_NLP_DA_TRIE_H_
#define _ILPLIB_NLP_DA_TRIE_H_

#include <string.h>
#include <string>
#include <vector>
#include "types.h"
#include "util/string/kstring.hpp"
#include "knlp/normalize.h"
#include "am/util/line_reader.h"

using namespace izenelib;
using namespace izenelib::util;
using namespace std;
namespace ilplib
{
namespace knlp
{
class DATrie
{
private:
    size_t max_length_;
    size_t tot_length_;
    size_t max_num;
    double MINVALUE_;
    typedef struct
    {
        KString kstr;
        double value;
        size_t len;
        KString to_word;
    } word_type;
    std::vector<int> base_;
    std::vector<int> check_;
    std::vector<double> value_;
    std::vector<size_t> cnt_;
    std::vector<word_type> dict_;

    template <class T, class R>
      T& AT(std::vector<T>& ar, R& i)
      {
          if ((size_t)i >= ar.size())
          {
              cout<<"resize datrie\n";
              ar.resize(ar.size()*2);
              assert(false);
          }
          return ar[(size_t)i];
      }

public:
    const size_t NOT_FOUND;
    static bool word_cmp(const word_type& x, const word_type& y)
    {
        return x.kstr < y.kstr;
    }

    DATrie():NOT_FOUND(-1) {}
    //mode=0:word value
    //mode=1:word
    //mode=2:word word
    DATrie(const std::string& file_name, const size_t mode = 0):NOT_FOUND(-1)
    {
        time_t time1, time2;
        time1 = clock();
        base_.clear();
        check_.clear();
        cnt_.clear();
        value_.clear();
        dict_.clear();
        max_length_ = 0;
        tot_length_ = 0;
        MINVALUE_ = 0.123;
        std::vector<word_type> tmpdict;
        tmpdict.clear();
        tmpdict.reserve(3456789);
        double value;

        char* st = NULL;
        izenelib::am::util::LineReader lr(file_name);
        while((st = lr.line(st)) != NULL)
        {

            if (mode == 2)//with synonym
            {
                if (strlen(st) == 0) continue;
                word_type tmpword;
                string s0 = string(st);
//                    Normalize::normalize(s0);
                int p = s0.find("\t",0);
                if (p <= 0 || p >= (int)s0.length()) continue;
                KString kstr(s0.substr(0,p));
                Normalize::normalize(kstr);
                KString to_word = KString(s0.substr(p+1, s0.length()-p-1).c_str());
                Normalize::normalize(to_word);
                if(kstr.length()==0 || to_word.length()==0) continue;
                tmpword.kstr = kstr;
                tmpword.to_word = to_word;
                tmpword.len = kstr.length();
                tmpdict.push_back(tmpword);
                tot_length_ += tmpword.len;
                max_length_ = std::max(max_length_, tmpword.len);
            }
            if (mode == 1)//only word
            {
                if (strlen(st) == 0) continue;
                word_type tmpword;
                string s0 = string(st);
                s0 += '\t';
//                    Normalize::normalize(s0);
                int p = s0.find("\t",0);
                if (p <= 0 || p >= (int)s0.length()) continue;
                KString kstr(s0.substr(0,p));
                Normalize::normalize(kstr);
                if (kstr.length()==0) continue;
                tmpword.kstr = kstr;
                tmpword.len = kstr.length();
                tmpdict.push_back(tmpword);
                tot_length_ += tmpword.len;
                max_length_ = std::max(max_length_, tmpword.len);
            }
            else if (mode == 0)//with score
            {
                if (strlen(st) == 0) continue;
                word_type tmpword;
                string s0 = string(st);
//                    Normalize::normalize(s0);
                int p = s0.find("\t",0);
                if (p <= 0 || p >= (int)s0.length()) continue;
                KString kstr(s0.substr(0,p));
                Normalize::normalize(kstr);
                if (kstr.length()==0) continue;
                value = atof(s0.substr(p+1, s0.length()-p-1).c_str());
                tmpword.kstr = kstr;
                tmpword.value = value;
                tmpword.len = kstr.length();
                tmpdict.push_back(tmpword);
                if (kstr == "[min]") MINVALUE_ = value;
                tot_length_ += tmpword.len;
                max_length_ = std::max(max_length_, tmpword.len);
            }
        }

        if (tmpdict.empty()) return;
        sort(tmpdict.begin(), tmpdict.end(), word_cmp);

        dict_.reserve(tmpdict.size());
        for(size_t i = 0; i+1 < tmpdict.size(); ++i)
            if (!(tmpdict[i].kstr == tmpdict[i+1].kstr))
                dict_.push_back(tmpdict[i]);
        dict_.push_back(tmpdict[tmpdict.size() - 1]);
//printf("word num = %zu, tot len = %zu, max len = %zu\n", dict_.size(), tot_length_, max_length_);

        max_num = tot_length_ * 3;
        max_num = std::max((size_t)1000000, max_num);
        time2 = clock();
//printf("before build dict time = %lf\n", (double)(time2 - time1) / 1000000);
        build(dict_);
//cout<<"build finish"<<endl;
        time1 = clock();
//printf("build %zu dict time = %lf\n", max_num, (double)(time1 - time2) / 1000000);
    }

    ~DATrie() {}

    void build(std::vector<word_type>& word)
    {
        if (word.empty()) return;
        std::vector<size_t> ad,next,pre;
        size_t lastad, tmpad, temp;
        size_t dataNum;
        size_t tryBase, tryBaseCount, tryBaseMax = 2;
        int start;
        size_t word_size = word.size();

        ad.resize(word_size);
        next.resize(word_size);
        pre.resize(word_size);

        base_.resize(max_num);
        check_.resize(max_num);
        cnt_.resize(max_num);
        value_.resize(max_num);
        for (size_t i = 0; i < word_size; ++i)
        {
            AT(ad,i) = AT(word,i).kstr[0];
            AT(base_,AT(ad,i)) = 1;
            AT(next,i) = i + 1;
            AT(pre,i) = i - 1;
        }
        tryBase = 1;
        tryBaseCount = 0;
        dataNum = 0;
        start = 0;
        for (size_t i = 0; i < max_length_; i++)
        {
            lastad = start;
            size_t j = 0;
            for (j = AT(next, start); j < word_size; j = AT(next, j))
            {
                size_t k = 0;
                for (k = 0; k <= i && AT(word, j).kstr[k] == AT(word, AT(pre, j)).kstr[k]; ++k);
                if (k <= i)
                {
                    size_t tmpBase = 0;
                    for (tmpBase = tryBase; tmpBase < max_num; ++tmpBase)
                    {
                        for (k = lastad; k < j; k = AT(next, k))
                        {
                            word_type tmpw = AT(word, k);
                            if (i+1 < tmpw.len)
                            {
                                size_t ind = tmpBase + tmpw.kstr[i+1];
                                if (AT(base_, ind) != 0)
                                    break;
                            }
                        }
                        if (k == j)
                            break;
                        ++AT(cnt_, tmpBase);
                    }

                    while (AT(cnt_, tryBase) >= tryBaseMax)
                        ++tryBase;

                    AT(base_, AT(ad, lastad)) = tmpBase;
                    for (k = lastad; k < j; k = AT(next, k))
                        if (i+1 < word[k].len)
                        {
                            temp = tmpBase + word[k].kstr[i+1];
                            AT(base_, temp) = 1;
                            AT(check_, temp) = AT(ad, k);
                            AT(ad, k) = temp;
                            dataNum = std::max(dataNum, temp);
                        }
                    lastad = j;
                }
            }

            size_t tmpBase = 0;
            for (tmpBase = tryBase; tmpBase < max_num; ++tmpBase)
            {
                size_t k = 0;
                for (k = lastad; k < j; k = AT(next, k))
                {
                            word_type tmpw = AT(word, k);
                            if (i+1 < tmpw.len)
                            {
                                size_t ind = tmpBase + tmpw.kstr[i+1];
                                if (AT(base_, ind) != 0)
                                    break;
                            }
                }
                if (k == j)
                    break;
            }

            AT(base_, AT(ad, lastad)) = tmpBase;
            for (size_t k = lastad; k < j; k = AT(next, k))
                if (i+1 < AT(word, k).len)
                {
                    temp = tmpBase + AT(word, k).kstr[i+1];
                    AT(base_, temp) = 1;
                    AT(check_, temp) = AT(ad, k);
                    AT(ad, k) = temp;
                    dataNum = std::max(dataNum, temp);
                }

            start = -1;
            for (size_t j = 0; j < word_size; ++j)
                if (AT(word, j).len > i+1)
                {
                    if (start == -1)
                    {
                        start = j;
                        lastad = j;
                        continue;
                    }
                    AT(next, lastad) = j;
                    AT(pre, j) = lastad;
                    lastad = j;
                }
            AT(next, lastad) = word_size;
        }

        for (size_t i = 0; i < word_size; ++i)
        {
            tmpad = AT(word, i).kstr[0];
            for (size_t j = 1; j < word[i].len; ++j)
                tmpad = abs(AT(base_, tmpad)) + AT(word, i).kstr[j];
            if (AT(base_, tmpad) > 0)
            {
                AT(base_, tmpad) = 0 - AT(base_, tmpad);
                AT(value_, tmpad) = i;
            }
        }

        {
            std::vector<size_t>().swap(cnt_);
            std::vector<size_t>().swap(ad);
            std::vector<size_t>().swap(next);
            std::vector<size_t>().swap(pre);
        }

        size_t tmp1=0;
        for(size_t i = max_num-1; i > 0; --i)
            if (check_[i]!=0 || base_[i]!=0 || value_[i]!=0 )
            {
                tmp1=i+1;
                break;
            }
        std::vector<int> t1;
        t1.resize(tmp1);
        memcpy(&t1[0], &check_[0], tmp1 * sizeof(int));
        check_.swap(t1);

        std::vector<int> t2;
        t2.resize(tmp1);
        memcpy(&t2[0], &base_[0], tmp1 * sizeof(int));
        base_.swap(t2);

        std::vector<double> t3;
        t3.resize(tmp1);
        memcpy(&t3[0], &value_[0], tmp1 * sizeof(double));
        value_.swap(t3);

    }

    KString id2word(const size_t id)
    {
        if (id >= dict_.size())return KString();
        KString r = AT(dict_, id).kstr;
        return KString(r.get_bytes(), r.get_bytes()+r.length());
    }

    size_t find_word(const KString& st, const bool normalize = 0)
    {
//                KString st(kst);
//                if (normalize)
//                    Normalize::normalize(st);
        int ad = 0, nextad = 0;
        size_t len = st.length();
        if (len == 0) return NOT_FOUND;
        for (size_t i = 0; i < len; ++i)
        {
            nextad = abs(AT(base_,ad)) + st[i];
            if ((size_t)nextad >= base_.size() || AT(base_,nextad) == 0 || AT(check_,nextad) != ad)
                return NOT_FOUND;
            ad = nextad;
        }
        if (AT(base_, ad) < 0)
            return AT(value_,ad);
        return NOT_FOUND;
    }

    size_t find_word(const KString& st, const size_t be, const size_t en)
    {
        int ad = 0, nextad = 0;
        size_t len = st.length();
        if (len == 0) return NOT_FOUND;
        for (size_t i = be; i < en; ++i)
        {
            nextad = abs(AT(base_, ad)) + st[i];
            if ((size_t)nextad >= base_.size() || AT(base_,nextad) == 0 || AT(check_,nextad) != ad)
                return NOT_FOUND;
            ad = nextad;
        }
        if (AT(base_,ad) < 0)
            return AT(value_,ad);
        return NOT_FOUND;
    }

    void find_syn(const KString& st, KString& syn)
    {
//                KString st(kst);
//                if (normalize)
//                    Normalize::normalize(st);
        int ad = 0, nextad = 0;
        size_t len = st.length();
        if (len == 0) return ;
        for (size_t i = 0; i < len; ++i)
        {
            nextad = abs(AT(base_, ad)) + st[i];
            if ((size_t)nextad >= base_.size() || AT(base_,nextad) == 0 || AT(check_,nextad) != ad)
                return ;
            ad = nextad;
        }
        if (AT(base_, ad) < 0)
        {
            KString to_word = AT(dict_,AT(value_,ad)).to_word;
            syn = KString(to_word.get_bytes(), to_word.get_bytes()+to_word.length());
        }
        return ;
    }

    bool check_term(const KString& st, const bool normalize = 0)
    {
//                KString st(kst);
//                if (normalize)
//                    Normalize::normalize(st);
        int ad = 0, nextad;
        size_t len = st.length();
        if (len == 0) return 0;
        for (size_t i = 0; i < len; ++i)
        {
            nextad = abs(AT(base_, ad)) + st[i];
            if ((size_t)nextad >= base_.size() || AT(base_,nextad) == 0 || AT(check_,nextad) != ad)
                return 0;
            ad = nextad;
        }
        if (AT(base_, ad) < 0)
            return 1;
        return 0;
    }

    double score(const KString& st, const bool normalize = 0)
    {
        size_t ind = find_word(st, normalize);
        if (ind != NOT_FOUND)
            return AT(dict_,ind).value;
        return MINVALUE_;
    }

    double score(const size_t ind)
    {
        if (ind >= dict_.size())
            return MINVALUE_;
        return AT(dict_, ind).value;
    }

    size_t size() const
    {
        return dict_.size();
    }

    double min() const
    {
        return MINVALUE_;
    }

    void token(const KString& st, std::vector<std::pair<KString, double> >& term)
    {
        size_t i = 0;
        size_t len = st.length();
        if (len == 0) return;
        size_t term_size = 0;
        term.resize(len);
        while(i < len)
        {
            int maxlen = -1, ad = 0, nextad, flag = 0;
            double value = MINVALUE_;
            for (size_t j = 0; j < len - i; ++j)
            {
                nextad = abs(AT(base_, ad)) + st[i+j];
                if ((size_t)nextad >= base_.size() || AT(base_,nextad) == 0 || AT(check_,nextad) != ad)
                {
                    flag = 0;
                    break;
                }
                ad = nextad;
                if (AT(base_, ad) < 0)
                {
                    maxlen = j+1;
                    value = value_[ad];
                }
            }

            if (maxlen == -1)
            {
                term[term_size].first = st.substr(i, 1);
                term[term_size++].second = MINVALUE_;
                i += 1;
            }
            else
            {
                term[term_size].first = st.substr(i, maxlen);
                term[term_size++].second = AT(dict_,value).value;
                i += maxlen;
            }
        }
        term.resize(term_size);
    }

    void token(const KString& st, const bool score, std::vector<KString>& term)
    {
        size_t i = 0;
        size_t len = st.length();
        if (len == 0) return;
        size_t term_size = 0;
        term.resize(len);
        while(i < len)
        {
            int maxlen = -1, ad = 0, nextad, flag = 0;
            for (size_t j = 0; j < len - i; ++j)
            {
                nextad = abs(AT(base_, ad)) + st[i+j];
                if ((size_t)nextad >= base_.size() || AT(base_,nextad) == 0 || AT(check_,nextad) != ad)
                {
                    flag = 0;
                    break;
                }
                ad = nextad;
                if (AT(base_, ad) < 0)
                {
                    maxlen = j+1;
                }
            }

            if (maxlen == -1)
            {
                term[term_size++] = st.substr(i, 1);
                i += 1;
            }
            else
            {
                term[term_size++] = st.substr(i, maxlen);
                i += maxlen;
            }
        }
        term.resize(term_size);
    }

    void sub_token(const KString& st, std::vector<std::pair<KString, double> >& term)
    {
        size_t r = find_word(st);
        if (r == NOT_FOUND) 
	{
            token(st, term);
            return;
        }
        else
        {
            size_t len = st.length();
            for (int32_t i = (int32_t)len-1; i > 0; --i)
            {
                size_t p = find_word(st, 0, i);
                size_t q = find_word(st, i, len);
                if (p != NOT_FOUND && q != NOT_FOUND)
                {
                    term.push_back(std::make_pair(st.substr(0, i), AT(dict_,p).value));
                    term.push_back(std::make_pair(st.substr(i, len - i), AT(dict_,q).value));
                    return;
                }
            }
            term.push_back(std::make_pair(st, AT(dict_,r).value));
        }
    }

    void sub_token(const KString& st, const bool score, std::vector<KString>& term)
    {
        size_t r = find_word(st);
        if (r == NOT_FOUND)
        {
            token(st, 0, term);
            return;
        }
        else
        {
            size_t len = st.length();
            for (int32_t i = (int32_t)len-1; i > 0; --i)
            {
                size_t p = find_word(st, 0, i);
                size_t q = find_word(st, i, len);
                if (p != NOT_FOUND && q != NOT_FOUND)
                {
                    term.push_back(st.substr(0, i));
                    term.push_back(st.substr(i, len - i));
                    return;
                }
            }
            term.push_back(st);
        }
    }

};
}
}
#endif


