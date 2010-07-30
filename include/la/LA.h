/**
 * @brief   Defines LA class
 * @file    LA.h
 * @date    2009.06.16
 * @author  MyungHyun Lee(Kent), originally zhjay
 * @details
 *  2009.10.25 - The original desing by Jay uses a pipeline approach to combine
 * a number of Analyzers to get a result. This approach is disabled - may be temporarily -
 * due to the fact that it obscures the definition of special, primary, secondary terms.
 * Especially the primary and secondary term. So, the pipeline approach is disabled.
 *
 */

#ifndef _LA_H_
#define _LA_H_

#include <la/stem/Stemmer.h>
#include <vector>
#include <la/Term.h>
#include <la/Analyzer.h>
#include <la/Tokenizer.h>
#include <la/EnglishAnalyzer.h>
#include <la/StemAnalyzer.h>
#include <la/NGramAnalyzer.h>
#include <la/MatrixAnalyzer.h>
#include <la/PlainDictionary.h>

#include <boost/smart_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <3rdparty/am/rde_hashmap/hash_map.h>

#include <la/MultiLanguageAnalyzer.h>
#include <la/CommonLanguageAnalyzer.h>

#ifdef USE_WISEKMA
    #include <la/KoreanLanguageAction.h>
#endif

#ifdef USE_IZENECMA
	#include <la/ChineseLanguageAction.h>
#endif

namespace la
{
    /**
     * @brief   LA class combines the Tokenizer and Analyzer to get analyzed results
     *          of a given text
     * @details
     *  Text -> [Tokenizer] -> tokens -> [Analyze] -> terms
     */
    class LA
    {

        private:
            Tokenizer               tokenizer_;
            shared_ptr<Analyzer>    analyzer_;

            unsigned int            TERM_LENGTH_THRESHOLD_;   // TODO: what is the maximum limit?
            bool                    bCaseSensitive_; // Whether is case sensitive

        public:
            typedef rde::hash_map<izenelib::util::UCS2Char, bool> PunctsType;

            LA();

            void setTokenizerConfig( const TokenizeConfig & tokenConfig )
            {
                tokenizer_.setConfig( tokenConfig );
            }

            void setAnalyzer( const boost::shared_ptr<Analyzer> & analyzer )
            {
                analyzer_ = analyzer;
                if( analyzer.get() == NULL )
                    bCaseSensitive_ = false;
                else
                    bCaseSensitive_ = analyzer.get()->getCaseSensitive();
            }

            boost::shared_ptr<Analyzer> getAnalyzer()
            {
                return analyzer_;
            }

            void setMaxTermLength( unsigned int length )
            {
                if( length > 0 )
                {
                    TERM_LENGTH_THRESHOLD_ = length;
                }
            }

            void process( UStringHashFunctor* hash,
                    const izenelib::util::UString & inputString,
                    TermIdList & outList );

            void process_index( const izenelib::util::UString & inputString, TermList & outList );

            void process_search( const izenelib::util::UString & inputString, TermList & outList );

            /// @param inputString      input string
            /// @param rawTerms         raw term list
            /// @param primaryTerms     primary term list
            /// @param outList          the complete output list generated by specific Analyzer
            /// @param index            calls process_index() if true, process_search() if false
            void process_index(
                    const izenelib::util::UString& inputString,
                    TermList & specialTermList,
                    TermList & primaryTermList,
                    TermList & outList
                    );

            void process_search(
                    const izenelib::util::UString& inputString,
                    TermList & specialTermList,
                    TermList & primaryTermList,
                    TermList & outList
                    );

            /**
             * Remove Stop Words in the termList
             * \param termList input and output list
             * \param stopDict stop word dictionary
             */
            static void removeStopwords(
                    TermList & termList,
                    shared_ptr<PlainDictionary>&  stopDict
                    );

        private:
            // TODO: A quick fix, and not a good way to do filtering. it's too slow.
            // Considering to apply filtering in Analyzers
            void lengthFilter( TermList & termList );

    };

    izenelib::util::UString toExpandedString( const TermList & termList );

}

#endif /* _LA_H_ */
