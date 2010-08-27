/**
 * @author Wei
 */

#ifndef _CHINESE_ANALYZER_H_
#define _CHINESE_ANALYZER_H_

#include <la/analyzer/CommonLanguageAnalyzer.h>

#include <cma_factory.h>
#include <knowledge.h>
#include <sentence.h>
#include <analyzer.h>

namespace la
{

class ChineseAnalyzer : public CommonLanguageAnalyzer
{
public:

    enum ChineseAnalysisType
    {
        maximum_entropy = 1,
        maximum_match = 2,
        minimum_match = 3,
    };

    ChineseAnalyzer( const std::string knowledgePath, bool loadModel = true );

    ~ChineseAnalyzer();

    void setIndexMode();

    void setLabelMode();

    void setAnalysisType( ChineseAnalysisType type )
    {
        pA_->setOption( cma::Analyzer::OPTION_ANALYSIS_TYPE, type );
    }

    void setNBest( unsigned int num=2 )
    {
        pA_->setOption( cma::Analyzer::OPTION_TYPE_NBEST, num );
    }

protected:

    inline void parse(const UString & input)
    {
        // Since cma accept a whole article as input,
        // it maybe extraordinary long sometimes.
        if(input_string_buffer_size_ < input.length()*3+1)
        {
            while(input_string_buffer_size_ < input.length()*3+1)
            {
                input_string_buffer_size_ *= 2;
            }
            delete input_string_buffer_;
            input_string_buffer_ = new char[input_string_buffer_size_];
        }

        input.convertString(izenelib::util::UString::UTF_8,
                            input_string_buffer_, input_string_buffer_size_);
        pS_->setString( input_string_buffer_ );
        pA_->runWithSentence( *pS_ );

        listIndex_ = 0;
        lexiconIndex_ = -1;
        localOffset_ = -1;

        resetToken();
    }

    inline bool nextToken()
    {
        while(doNext())
        {
            nativeToken_ = pS_->getLexicon(listIndex_, lexiconIndex_);
            nativeTokenLen_ = strlen(nativeToken_);

            if(nativeTokenLen_ > term_ustring_buffer_limit_ )
                continue;

            token_ = output_ustring_buffer_;
            len_ = UString::toUcs2(izenelib::util::UString::UTF_8,
                                   nativeToken_, nativeTokenLen_, output_ustring_buffer_,
                                   term_ustring_buffer_limit_);

            morpheme_ = pS_->getPOS(listIndex_, lexiconIndex_);
            pos_ = pS_->getStrPOS(listIndex_, lexiconIndex_);
            offset_ = localOffset_;
            isIndex_ = pS_->isIndexWord(listIndex_, lexiconIndex_);
            return true;
        }
        resetToken();
        return false;
    }

    inline bool isAlpha()
    {
        return morpheme_ == flMorp_;
    }

    inline bool isSpecialChar()
    {
        return morpheme_ == scMorp_;
    }

private:

    inline bool doNext()
    {
        if(listIndex_ == pS_->getListSize())
        {
            return false;
        }

        ++ localOffset_;
        ++ lexiconIndex_;

        while(lexiconIndex_ == pS_->getCount(listIndex_))
        {
            ++ listIndex_;
            lexiconIndex_ = 0;
            localOffset_ = 0;
            if(listIndex_ == pS_->getListSize()) return false;
        }
        return true;
    }

    inline void resetAnalyzer();

    inline void addDefaultPOSList( vector<string>& posList );

private:

    cma::Analyzer * pA_;

    cma::Sentence * pS_;

    size_t input_string_buffer_size_;
    char * input_string_buffer_;

    UString::CharT * output_ustring_buffer_;

    int localOffset_;

    int listIndex_;

    int lexiconIndex_;

    unsigned int morpheme_;

    unsigned int flMorp_;

    unsigned int scMorp_;
};

}

#endif
