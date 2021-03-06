/**
 * @file    CommonLanguageAnalyzer.cpp
 * @author  Kent, Vernkin
 * @date    Nov 23, 2009
 * @details
 *  Common Language Analyzer for Chinese/Japanese/Korean.
 */

/**
 * @brief Rewrite CLA using new interfaces. Add English.
 * @author Wei
 */

#ifndef COMMONLANGUAGEANALYZER_H_
#define COMMONLANGUAGEANALYZER_H_

#include <la/analyzer/Analyzer.h>
#include <la/dict/SingletonDictionary.h>
#include <la/dict/UpdatableSynonymContainer.h>
#include <la/dict/UpdateDictThread.h>
#include <am/vsynonym/VSynonym.h>

#include <la/stem/Stemmer.h>

#include <util/ustring/UString.h>

namespace la
{

class CommonLanguageAnalyzer : public Analyzer
{
public:

    CommonLanguageAnalyzer();

    CommonLanguageAnalyzer( const std::string & synonymDictPath,
        UString::EncodingType synonymEncode);

    ~CommonLanguageAnalyzer();

    virtual void setCaseSensitive(bool casesensitive = true, bool containlower = true)
    {
        bCaseSensitive_ = casesensitive;
        bContainLower_ = containlower;
    };

    virtual void setExtractEngStem( bool extractEngStem = true )
    {
        bExtractEngStem_ = extractEngStem;
    }

    virtual void setExtractSynonym( bool extractSynonym = true)
    {
        bExtractSynonym_ = extractSynonym;
    }

    void setSynonymUpdateInterval(unsigned int seconds);

public:
    virtual void setRemoveStopwords(bool removeStopwords = true)
    {
        bRemoveStopwords_ = removeStopwords;
    }

    bool isRemoveStopwords()
    {
        return bRemoveStopwords_;
    }

    /// obsolete
    virtual void analyzeSynonym(TermList& outList, size_t n);

protected:
    /// obsolete
    bool getSynonym(
            const UString& combine,
            int offset,
            const unsigned char andOrBit,
            const unsigned int level,
            TermList& syOutList,
            unsigned int& subLevel);

protected:

    /// Parse given input
    virtual void parse(const UString & input ) = 0;

    /// Fill token_, len_, offset_
    virtual bool nextToken() = 0;

    /// whether morpheme_ indicates foreign language
    virtual bool isAlpha() = 0;

    /// whether morpheme_ indicates special character, e.g. punctuations
    virtual bool isSpecialChar() = 0;

    /// whether current token is stopword
    virtual bool isStopword() { return false; }

    inline void resetToken()
    {
        token_ = NULL;
        len_ = 0;
        nativeToken_ = NULL;
        nativeTokenLen_ = 0;
        offset_ = 0;
        pos_ = NULL;
        level_ = 0;
        isIndex_ = false;
        isRaw_ = false;
    }

protected:

    virtual int analyze_impl( const Term& input, void* data, HookType func );

    virtual bool analyze_synonym_impl(const izenelib::util::UString& inputString,  SynonymOutputType& synonymOutput);

protected:

    izenelib::am::VSynonymContainer*      pSynonymContainer_;
    izenelib::am::VSynonym*               pSynonymResult_;
    izenelib::util::UString::EncodingType synonymEncode_;
    boost::shared_ptr<UpdatableSynonymContainer> uscSPtr_;

    stem::Stemmer *                     pStemmer_;

    static const size_t term_ustring_buffer_limit_ = 4096;
    static const size_t term_string_buffer_limit_ = 4096*3;

    char * lowercase_string_buffer_;
    UString::CharT * lowercase_ustring_buffer_;
    UString::CharT * synonym_ustring_buffer_;
    UString::CharT * stemming_ustring_buffer_;

    const UString::CharT * token_;
    size_t len_;
    const char * nativeToken_;
    size_t nativeTokenLen_;
    int offset_;
    int localOffset_;
    const char* pos_;
    int level_;
    bool isIndex_;
    bool isRaw_;

    bool bCaseSensitive_;

    bool bContainLower_;

    bool bExtractEngStem_;

    bool bExtractSynonym_;

    bool bChinese_;

    bool bRemoveStopwords_;
};

}

#endif /* COMMONLANGUAGEANALYZER_H_ */
