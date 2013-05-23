/*
 * =====================================================================================
 *
 *       Filename:  normalize.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013年05月22日 12时04分42秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Kevin Hu (), kevin.hu@b5m.com
 *        Company:  B5M.com
 *
 * =====================================================================================
 */
#ifndef _ILPLIB_NLP_NORMALIZE_H_
#define _ILPLIB_NLP_NORMALIZE_H_

#include "trd2simp.h"
#include "util/string/kstring.hpp"

namespace ilplib{
	namespace knlp{

class Normalize
{
	static Trad2Simp trd2smp_;
	public:
		static void normalize(izenelib::util::KString& kstr)
		{
			kstr.to_dbc();
			kstr.to_lower_case();
			trd2smp_.transform(kstr);
		}
};

Trad2Simp Normalize::trd2smp_;

}}//namespace
#endif
