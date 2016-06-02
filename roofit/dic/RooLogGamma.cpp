// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME dIafsdIcerndOchdIuserdIpdIplucadIrepositoriesdIlhcbbhamdItoolsdIroofitdIdicdIRooLogGamma

/*******************************************************************/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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
#include "/afs/cern.ch/user/p/pluca/repositories/lhcbbham/tools/roofit/RooLogGamma.cpp"

// Header files passed via #pragma extra_include

namespace ROOT {
   static void *new_RooLogGamma(void *p = 0);
   static void *newArray_RooLogGamma(Long_t size, void *p);
   static void delete_RooLogGamma(void *p);
   static void deleteArray_RooLogGamma(void *p);
   static void destruct_RooLogGamma(void *p);
   static void streamer_RooLogGamma(TBuffer &buf, void *obj);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::RooLogGamma*)
   {
      ::RooLogGamma *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::RooLogGamma >(0);
      static ::ROOT::TGenericClassInfo 
         instance("RooLogGamma", ::RooLogGamma::Class_Version(), "roofit/RooLogGamma.h", 16,
                  typeid(::RooLogGamma), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::RooLogGamma::Dictionary, isa_proxy, 16,
                  sizeof(::RooLogGamma) );
      instance.SetNew(&new_RooLogGamma);
      instance.SetNewArray(&newArray_RooLogGamma);
      instance.SetDelete(&delete_RooLogGamma);
      instance.SetDeleteArray(&deleteArray_RooLogGamma);
      instance.SetDestructor(&destruct_RooLogGamma);
      instance.SetStreamerFunc(&streamer_RooLogGamma);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::RooLogGamma*)
   {
      return GenerateInitInstanceLocal((::RooLogGamma*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_(Init) = GenerateInitInstanceLocal((const ::RooLogGamma*)0x0); R__UseDummy(_R__UNIQUE_(Init));
} // end of namespace ROOT

//______________________________________________________________________________
atomic_TClass_ptr RooLogGamma::fgIsA(0);  // static to hold class pointer

//______________________________________________________________________________
const char *RooLogGamma::Class_Name()
{
   return "RooLogGamma";
}

//______________________________________________________________________________
const char *RooLogGamma::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::RooLogGamma*)0x0)->GetImplFileName();
}

//______________________________________________________________________________
int RooLogGamma::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::RooLogGamma*)0x0)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *RooLogGamma::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::RooLogGamma*)0x0)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *RooLogGamma::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD2(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::RooLogGamma*)0x0)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
void RooLogGamma::Streamer(TBuffer &R__b)
{
   // Stream an object of class RooLogGamma.

   UInt_t R__s, R__c;
   if (R__b.IsReading()) {
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c); if (R__v) { }
      RooAbsPdf::Streamer(R__b);
      x.Streamer(R__b);
      nu.Streamer(R__b);
      landa.Streamer(R__b);
      alpha.Streamer(R__b);
      R__b.CheckByteCount(R__s, R__c, RooLogGamma::IsA());
   } else {
      R__c = R__b.WriteVersion(RooLogGamma::IsA(), kTRUE);
      RooAbsPdf::Streamer(R__b);
      x.Streamer(R__b);
      nu.Streamer(R__b);
      landa.Streamer(R__b);
      alpha.Streamer(R__b);
      R__b.SetByteCount(R__c, kTRUE);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_RooLogGamma(void *p) {
      return  p ? new(p) ::RooLogGamma : new ::RooLogGamma;
   }
   static void *newArray_RooLogGamma(Long_t nElements, void *p) {
      return p ? new(p) ::RooLogGamma[nElements] : new ::RooLogGamma[nElements];
   }
   // Wrapper around operator delete
   static void delete_RooLogGamma(void *p) {
      delete ((::RooLogGamma*)p);
   }
   static void deleteArray_RooLogGamma(void *p) {
      delete [] ((::RooLogGamma*)p);
   }
   static void destruct_RooLogGamma(void *p) {
      typedef ::RooLogGamma current_t;
      ((current_t*)p)->~current_t();
   }
   // Wrapper around a custom streamer member function.
   static void streamer_RooLogGamma(TBuffer &buf, void *obj) {
      ((::RooLogGamma*)obj)->::RooLogGamma::Streamer(buf);
   }
} // end of namespace ROOT for class ::RooLogGamma

namespace {
  void TriggerDictionaryInitialization_RooLogGamma_Impl() {
    static const char* headers[] = {
"roofit/RooLogGamma.cpp",
0
    };
    static const char* includePaths[] = {
"/afs/cern.ch/sw/lcg/releases/ROOT/6.06.02-6cc9c/x86_64-slc6-gcc49-opt/include",
"/afs/cern.ch/user/p/pluca/repositories/lhcbbham/tools/",
0
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "RooLogGamma dictionary forward declarations' payload"
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_Autoloading_Map;
class __attribute__((annotate(R"ATTRDUMP(Your description goes here...)ATTRDUMP"))) __attribute__((annotate("$clingAutoload$roofit/RooLogGamma.cpp")))  RooLogGamma;
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "RooLogGamma dictionary payload"

#ifndef G__VECTOR_HAS_CLASS_ITERATOR
  #define G__VECTOR_HAS_CLASS_ITERATOR 1
#endif

#define _BACKWARD_BACKWARD_WARNING_H
#include "roofit/RooLogGamma.cpp"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[]={
"RooLogGamma", payloadCode, "@",
nullptr};

    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("RooLogGamma",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_RooLogGamma_Impl, {}, classesHeaders);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_RooLogGamma_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_RooLogGamma() {
  TriggerDictionaryInitialization_RooLogGamma_Impl();
}
