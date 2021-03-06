/** \file analyzer_impl.cpp
 * Implementation of class AnalyzerImpl.
 * 
 * \author Jun Jiang
 * \version 0.1
 * \date Dec 24, 2009
 */

#include "analyzer_impl.h"
#include "knowledge_impl.h"
#include "sentence_tokenizer.h"
#include "language_analyzer.h"
#include "encoding_analyzer.h"
#include "profile_model.h"

#include <cstring> // strlen
#include <cassert>

NS_ILPLIB_LANGID_BEGIN

AnalyzerImpl::AnalyzerImpl()
    : knowledge_(0), sentenceTokenizer_(0), languageAnalyzer_(0), encodingAnalyzer_(0)
{
}

AnalyzerImpl::~AnalyzerImpl()
{
    clear();
}

void AnalyzerImpl::clear()
{
    delete sentenceTokenizer_;
    delete languageAnalyzer_;
    delete encodingAnalyzer_;
}

void AnalyzerImpl::setKnowledge(Knowledge* pKnowledge)
{
    assert(pKnowledge);

    // release the resources before allocating new ones
    clear();

    knowledge_ = dynamic_cast<KnowledgeImpl*>(pKnowledge);
    assert(knowledge_);

    const SentenceBreakTable& breakTable = knowledge_->getSentenceBreakTable();
    const ScriptTable& scriptTable = knowledge_->getScriptTable();
    const ProfileModel& profileModel = knowledge_->getProfileModel();

    sentenceTokenizer_ = new SentenceTokenizer(breakTable);
    languageAnalyzer_ = new LanguageAnalyzer(scriptTable, breakTable);
    encodingAnalyzer_ = new EncodingAnalyzer(profileModel);

    languageAnalyzer_->setOptionSrc(this);
}

bool AnalyzerImpl::encodingFromString(const char* str, EncodingID& id)
{
    return encodingAnalyzer_->encodingFromString(str, id, getOption(Analyzer::OPTION_TYPE_LIMIT_ANALYZE_SIZE));
}

bool AnalyzerImpl::encodingFromFile(const char* fileName, EncodingID& id)
{
    return encodingAnalyzer_->encodingFromFile(fileName, id, getOption(Analyzer::OPTION_TYPE_LIMIT_ANALYZE_SIZE));
}

std::size_t AnalyzerImpl::sentenceLength(const char* str)
{
    return sentenceTokenizer_->getSentenceLength<ENCODING_ID_UTF8>(str, str+strlen(str));
}

std::size_t AnalyzerImpl::sentenceLength(const izenelib::util::UString& ustr, std::size_t pos)
{
    const char* begin = reinterpret_cast<const char*>(ustr.data());
    const char* end = begin + ustr.size();

    // 2 bytes for each character
    begin += (pos << 1);
    std::size_t byteCount = sentenceTokenizer_->getSentenceLength<ENCODING_ID_UTF16>(begin, end);

    return (byteCount >> 1);
}

bool AnalyzerImpl::languageFromString(const char* str, LanguageID& id)
{
    return languageAnalyzer_->primaryIDFromString(str, id, getOption(Analyzer::OPTION_TYPE_LIMIT_ANALYZE_SIZE));
}

bool AnalyzerImpl::languageFromString(const izenelib::util::UString& ustr, LanguageID& id)
{
    return languageAnalyzer_->primaryIDFromString(ustr, id, getOption(Analyzer::OPTION_TYPE_LIMIT_ANALYZE_SIZE));
}

bool AnalyzerImpl::languageFromFile(const char* fileName, LanguageID& id)
{
    return languageAnalyzer_->primaryIDFromFile(fileName, id, getOption(Analyzer::OPTION_TYPE_LIMIT_ANALYZE_SIZE));
}

bool AnalyzerImpl::languageListFromString(const char* str, std::vector<LanguageID>& idVec)
{
    return languageAnalyzer_->multipleIDFromString(str, idVec);
}

bool AnalyzerImpl::languageListFromString(const izenelib::util::UString& ustr, std::vector<LanguageID>& idVec)
{
    return languageAnalyzer_->multipleIDFromString(ustr, idVec);
}

bool AnalyzerImpl::languageListFromFile(const char* fileName, std::vector<LanguageID>& idVec)
{
    return languageAnalyzer_->multipleIDFromFile(fileName, idVec);
}

bool AnalyzerImpl::segmentString(const char* str, std::vector<LanguageRegion>& regionVec)
{
    return languageAnalyzer_->segmentString(str, regionVec);
}

bool AnalyzerImpl::segmentString(const izenelib::util::UString& ustr, std::vector<LanguageRegion>& regionVec)
{
    return languageAnalyzer_->segmentString(ustr, regionVec);
}

bool AnalyzerImpl::segmentFile(const char* fileName, std::vector<LanguageRegion>& regionVec)
{
    return languageAnalyzer_->segmentFile(fileName, regionVec);
}

NS_ILPLIB_LANGID_END
