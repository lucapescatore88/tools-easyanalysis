// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME dIafsdIcerndOchdIuserdIpdIplucadIrepositoriesdIlhcbbhamdItoolsdIroofitdIdicdIRooAmorosoPdf

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
#include "/afs/cern.ch/user/p/pluca/repositories/lhcbbham/tools/roofit/RooAmorosoPdf.cpp"

// Header files passed via #pragma extra_include

namespace ROOT {
   static void *new_RooAmorosoPdf(void *p = 0);
   static void *newArray_RooAmorosoPdf(Long_t size, void *p);
   static void delete_RooAmorosoPdf(void *p);
   static void deleteArray_RooAmorosoPdf(void *p);
   static void destruct_RooAmorosoPdf(void *p);
   static void streamer_RooAmorosoPdf(TBuffer &buf, void *obj);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::RooAmorosoPdf*)
   {
      ::RooAmorosoPdf *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::RooAmorosoPdf >(0);
      static ::ROOT::TGenericClassInfo 
         instance("RooAmorosoPdf", ::RooAmorosoPdf::Class_Version(), "roofit/RooAmorosoPdf.h", 16,
                  typeid(::RooAmorosoPdf), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::RooAmorosoPdf::Dictionary, isa_proxy, 16,
                  sizeof(::RooAmorosoPdf) );
      instance.SetNew(&new_RooAmorosoPdf);
      instance.SetNewArray(&newArray_RooAmorosoPdf);
      instance.SetDelete(&delete_RooAmorosoPdf);
      instance.SetDeleteArray(&deleteArray_RooAmorosoPdf);
      instance.SetDestructor(&destruct_RooAmorosoPdf);
      instance.SetStreamerFunc(&streamer_RooAmorosoPdf);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::RooAmorosoPdf*)
   {
      return GenerateInitInstanceLocal((::RooAmorosoPdf*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_(Init) = GenerateInitInstanceLocal((const ::RooAmorosoPdf*)0x0); R__UseDummy(_R__UNIQUE_(Init));
} // end of namespace ROOT

//______________________________________________________________________________
atomic_TClass_ptr RooAmorosoPdf::fgIsA(0);  // static to hold class pointer

//______________________________________________________________________________
const char *RooAmorosoPdf::Class_Name()
{
   return "RooAmorosoPdf";
}

//______________________________________________________________________________
const char *RooAmorosoPdf::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::RooAmorosoPdf*)0x0)->GetImplFileName();
}

//______________________________________________________________________________
int RooAmorosoPdf::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::RooAmorosoPdf*)0x0)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *RooAmorosoPdf::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::RooAmorosoPdf*)0x0)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *RooAmorosoPdf::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD2(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::RooAmorosoPdf*)0x0)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
void RooAmorosoPdf::Streamer(TBuffer &R__b)
{
   // Stream an object of class RooAmorosoPdf.

   UInt_t R__s, R__c;
   if (R__b.IsReading()) {
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c); if (R__v) { }
      RooAbsPdf::Streamer(R__b);
      x.Streamer(R__b);
      a.Streamer(R__b);
      theta.Streamer(R__b);
      alpha.Streamer(R__b);
      beta.Streamer(R__b);
      R__b.CheckByteCount(R__s, R__c, RooAmorosoPdf::IsA());
   } else {
      R__c = R__b.WriteVersion(RooAmorosoPdf::IsA(), kTRUE);
      RooAbsPdf::Streamer(R__b);
      x.Streamer(R__b);
      a.Streamer(R__b);
      theta.Streamer(R__b);
      alpha.Streamer(R__b);
      beta.Streamer(R__b);
      R__b.SetByteCount(R__c, kTRUE);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_RooAmorosoPdf(void *p) {
      return  p ? new(p) ::RooAmorosoPdf : new ::RooAmorosoPdf;
   }
   static void *newArray_RooAmorosoPdf(Long_t nElements, void *p) {
      return p ? new(p) ::RooAmorosoPdf[nElements] : new ::RooAmorosoPdf[nElements];
   }
   // Wrapper around operator delete
   static void delete_RooAmorosoPdf(void *p) {
      delete ((::RooAmorosoPdf*)p);
   }
   static void deleteArray_RooAmorosoPdf(void *p) {
      delete [] ((::RooAmorosoPdf*)p);
   }
   static void destruct_RooAmorosoPdf(void *p) {
      typedef ::RooAmorosoPdf current_t;
      ((current_t*)p)->~current_t();
   }
   // Wrapper around a custom streamer member function.
   static void streamer_RooAmorosoPdf(TBuffer &buf, void *obj) {
      ((::RooAmorosoPdf*)obj)->::RooAmorosoPdf::Streamer(buf);
   }
} // end of namespace ROOT for class ::RooAmorosoPdf

namespace {
  void TriggerDictionaryInitialization_RooAmorosoPdf_Impl() {
    static const char* headers[] = {
"roofit/RooAmorosoPdf.cpp",
0
    };
    static const char* includePaths[] = {
"/afs/cern.ch/sw/lcg/releases/ROOT/6.06.02-6cc9c/x86_64-slc6-gcc49-opt/include",
"/afs/cern.ch/user/p/pluca/repositories/lhcbbham/tools/",
0
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "RooAmorosoPdf dictionary forward declarations' payload"
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_Autoloading_Map;
class __attribute__((annotate(R"ATTRDUMP(Your description goes here...)ATTRDUMP"))) __attribute__((annotate("$clingAutoload$roofit/RooAmorosoPdf.cpp")))  RooAmorosoPdf;
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "RooAmorosoPdf dictionary payload"

#ifndef G__VECTOR_HAS_CLASS_ITERATOR
  #define G__VECTOR_HAS_CLASS_ITERATOR 1
#endif

#define _BACKWARD_BACKWARD_WARNING_H
#include "roofit/RooAmorosoPdf.cpp"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[]={
"RooAmorosoPdf", payloadCode, "@",
nullptr};

    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("RooAmorosoPdf",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_RooAmorosoPdf_Impl, {}, classesHeaders);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_RooAmorosoPdf_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_RooAmorosoPdf() {
  TriggerDictionaryInitialization_RooAmorosoPdf_Impl();
}
