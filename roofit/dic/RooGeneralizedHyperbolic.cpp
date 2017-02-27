// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME dIafsdIcerndOchdIuserdIpdIplucadIrepositoriesdItoolsmIeasyanalysisdIroofitdIdicdIRooGeneralizedHyperbolic

/*******************************************************************/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define G__DICTIONARY
#include "RConfig.h"
#include "TClass.h"
#include "TDictAttributeMap.h"
#include "TInterpreter.h"
#include "TROOT.h"
#include "TBuffer.h"
#include "TMemberInspector.h"
#include "TInterpreter.h"
#include "TVirtualMutex.h"
#include "TError.h"

#ifndef G__ROOT
#define G__ROOT
#endif

#include "RtypesImp.h"
#include "TIsAProxy.h"
#include "TFileMergeInfo.h"
#include <algorithm>
#include "TCollectionProxyInfo.h"
/*******************************************************************/

#include "TDataMember.h"

// Since CINT ignores the std namespace, we need to do so in this file.
namespace std {} using namespace std;

// Header files passed as explicit arguments
#include "/afs/cern.ch/user/p/pluca/repositories/tools-easyanalysis/roofit/RooGeneralizedHyperbolic.cpp"

// Header files passed via #pragma extra_include

namespace ROOT {
   static void *new_RooGeneralizedHyperbolic(void *p = 0);
   static void *newArray_RooGeneralizedHyperbolic(Long_t size, void *p);
   static void delete_RooGeneralizedHyperbolic(void *p);
   static void deleteArray_RooGeneralizedHyperbolic(void *p);
   static void destruct_RooGeneralizedHyperbolic(void *p);
   static void streamer_RooGeneralizedHyperbolic(TBuffer &buf, void *obj);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::RooGeneralizedHyperbolic*)
   {
      ::RooGeneralizedHyperbolic *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::RooGeneralizedHyperbolic >(0);
      static ::ROOT::TGenericClassInfo 
         instance("RooGeneralizedHyperbolic", ::RooGeneralizedHyperbolic::Class_Version(), "roofit/RooGeneralizedHyperbolic.h", 16,
                  typeid(::RooGeneralizedHyperbolic), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::RooGeneralizedHyperbolic::Dictionary, isa_proxy, 16,
                  sizeof(::RooGeneralizedHyperbolic) );
      instance.SetNew(&new_RooGeneralizedHyperbolic);
      instance.SetNewArray(&newArray_RooGeneralizedHyperbolic);
      instance.SetDelete(&delete_RooGeneralizedHyperbolic);
      instance.SetDeleteArray(&deleteArray_RooGeneralizedHyperbolic);
      instance.SetDestructor(&destruct_RooGeneralizedHyperbolic);
      instance.SetStreamerFunc(&streamer_RooGeneralizedHyperbolic);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::RooGeneralizedHyperbolic*)
   {
      return GenerateInitInstanceLocal((::RooGeneralizedHyperbolic*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_(Init) = GenerateInitInstanceLocal((const ::RooGeneralizedHyperbolic*)0x0); R__UseDummy(_R__UNIQUE_(Init));
} // end of namespace ROOT

//______________________________________________________________________________
atomic_TClass_ptr RooGeneralizedHyperbolic::fgIsA(0);  // static to hold class pointer

//______________________________________________________________________________
const char *RooGeneralizedHyperbolic::Class_Name()
{
   return "RooGeneralizedHyperbolic";
}

//______________________________________________________________________________
const char *RooGeneralizedHyperbolic::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::RooGeneralizedHyperbolic*)0x0)->GetImplFileName();
}

//______________________________________________________________________________
int RooGeneralizedHyperbolic::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::RooGeneralizedHyperbolic*)0x0)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *RooGeneralizedHyperbolic::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::RooGeneralizedHyperbolic*)0x0)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *RooGeneralizedHyperbolic::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD2(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::RooGeneralizedHyperbolic*)0x0)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
void RooGeneralizedHyperbolic::Streamer(TBuffer &R__b)
{
   // Stream an object of class RooGeneralizedHyperbolic.

   UInt_t R__s, R__c;
   if (R__b.IsReading()) {
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c); if (R__v) { }
      RooAbsPdf::Streamer(R__b);
      x.Streamer(R__b);
      l.Streamer(R__b);
      alpha.Streamer(R__b);
      beta.Streamer(R__b);
      delta.Streamer(R__b);
      mu.Streamer(R__b);
      R__b.CheckByteCount(R__s, R__c, RooGeneralizedHyperbolic::IsA());
   } else {
      R__c = R__b.WriteVersion(RooGeneralizedHyperbolic::IsA(), kTRUE);
      RooAbsPdf::Streamer(R__b);
      x.Streamer(R__b);
      l.Streamer(R__b);
      alpha.Streamer(R__b);
      beta.Streamer(R__b);
      delta.Streamer(R__b);
      mu.Streamer(R__b);
      R__b.SetByteCount(R__c, kTRUE);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_RooGeneralizedHyperbolic(void *p) {
      return  p ? new(p) ::RooGeneralizedHyperbolic : new ::RooGeneralizedHyperbolic;
   }
   static void *newArray_RooGeneralizedHyperbolic(Long_t nElements, void *p) {
      return p ? new(p) ::RooGeneralizedHyperbolic[nElements] : new ::RooGeneralizedHyperbolic[nElements];
   }
   // Wrapper around operator delete
   static void delete_RooGeneralizedHyperbolic(void *p) {
      delete ((::RooGeneralizedHyperbolic*)p);
   }
   static void deleteArray_RooGeneralizedHyperbolic(void *p) {
      delete [] ((::RooGeneralizedHyperbolic*)p);
   }
   static void destruct_RooGeneralizedHyperbolic(void *p) {
      typedef ::RooGeneralizedHyperbolic current_t;
      ((current_t*)p)->~current_t();
   }
   // Wrapper around a custom streamer member function.
   static void streamer_RooGeneralizedHyperbolic(TBuffer &buf, void *obj) {
      ((::RooGeneralizedHyperbolic*)obj)->::RooGeneralizedHyperbolic::Streamer(buf);
   }
} // end of namespace ROOT for class ::RooGeneralizedHyperbolic

namespace {
  void TriggerDictionaryInitialization_RooGeneralizedHyperbolic_Impl() {
    static const char* headers[] = {
"roofit/RooGeneralizedHyperbolic.cpp",
0
    };
    static const char* includePaths[] = {
"/afs/cern.ch/sw/lcg/releases/ROOT/6.08.02-99084/x86_64-slc6-gcc49-opt/include",
"/afs/cern.ch/user/p/pluca/repositories/tools-easyanalysis/",
0
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "RooGeneralizedHyperbolic dictionary forward declarations' payload"
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_Autoloading_Map;
class __attribute__((annotate(R"ATTRDUMP(Your description goes here...)ATTRDUMP"))) __attribute__((annotate("$clingAutoload$roofit/RooGeneralizedHyperbolic.cpp")))  RooGeneralizedHyperbolic;
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "RooGeneralizedHyperbolic dictionary payload"

#ifndef G__VECTOR_HAS_CLASS_ITERATOR
  #define G__VECTOR_HAS_CLASS_ITERATOR 1
#endif

#define _BACKWARD_BACKWARD_WARNING_H
#include "roofit/RooGeneralizedHyperbolic.cpp"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[]={
"RooGeneralizedHyperbolic", payloadCode, "@",
nullptr};

    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("RooGeneralizedHyperbolic",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_RooGeneralizedHyperbolic_Impl, {}, classesHeaders);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_RooGeneralizedHyperbolic_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_RooGeneralizedHyperbolic() {
  TriggerDictionaryInitialization_RooGeneralizedHyperbolic_Impl();
}
