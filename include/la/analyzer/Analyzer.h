/**
 * @brief   Defines Analyzerclass
 * @file    Analyzer.h
 * @author  zhjay, MyungHyun Lee (Kent)
 * @date    2009.06.10
 * @details
 *  2009.08.02 - Adding new interfaces
 */

/**
 * @brief   Embed IDManager in LA to accelerate the speed of indexing,
 *          Merging different interfaces into one.
 * @author  Wei
 * @date    2010.08.23
 */

#ifndef _LA_ANALYZER_H_
#define _LA_ANALYZER_H_

#include <la/common/Term.h>
#include <la/tokenizer/Tokenizer.h>

#include <util/ustring/UString.h>

namespace la
{

class MultiLanguageAnalyzer;

///
/// \brief interface of Analyzer
/// This class analyze terms according to the specific types of analyzer
///
class Analyzer
{

public:

    Analyzer() {}

    virtual ~Analyzer() {}

    void setTokenizerConfig( const TokenizeConfig & tokenConfig )
    {
        tokenizer_.setConfig( tokenConfig );
    }

    int analyze(const Term & input, TermList & output)
    {
        return analyze_impl(input, &output, &Analyzer::appendTermList);
    }

    template <typename IDManagerType>
    int analyze( IDManagerType* idm, const Term & input, TermIdList & output)
    {
        pair<TermIdList*, IDManagerType*> env = make_pair(&output, idm);
        return analyze_impl(input, &env, &Analyzer::appendTermIdList<IDManagerType>);
    }

protected:

    friend class MultiLanguageAnalyzer;

    typedef void (*HookType) (
        void* data,
        const UString::CharT* text,
        const size_t len,
        const unsigned int offset,
        const char * pos,
        const unsigned char andOrBit,
        const unsigned int level );

    template<typename IDManagerType>
    static void appendTermIdList(
        void* data,
        const UString::CharT* text,
        const size_t len,
        const unsigned int offset,
        const char * pos,
        const unsigned char andOrBit,
        const unsigned int level )
    {
        TermIdList * output = ((std::pair<TermIdList*, IDManagerType*>* ) data)->first;
        IDManagerType * idm = ((std::pair<TermIdList*, IDManagerType*>* ) data)->second;

        output->add(idm, text, len, offset);
    }

    static void appendTermList(
        void* data,
        const UString::CharT* text,
        const size_t len,
        const unsigned int offset,
        const char * pos,
        const unsigned char andOrBit,
        const unsigned int level)
    {
        TermList * output = (TermList *) data;
        output->add( text, len, offset, pos, andOrBit, level );
    }

    virtual int analyze_impl( const Term& input, void* data, HookType func ) = 0;

protected:

    Tokenizer tokenizer_;

};
}

#endif /* _LA_ANALYZER_H_ */
