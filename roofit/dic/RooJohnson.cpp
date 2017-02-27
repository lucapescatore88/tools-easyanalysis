// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME dIafsdIcerndOchdIuserdIpdIplucadIrepositoriesdItoolsmIeasyanalysisdIroofitdIdicdIRooJohnson

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
#include "/afs/cern.ch/user/p/pluca/repositories/tools-easyanalysis/roofit/RooJohnson.cpp"

// Header files passed via #pragma extra_include

namespace ROOT {
   static void *new_RooJohnson(void *p = 0);
   static void *newArray_RooJohnson(Long_t size, void *p);
   static void delete_RooJohnson(void *p);
   static void deleteArray_RooJohnson(void *p);
   static void destruct_RooJohnson(void *p);
   static void streamer_RooJohnson(TBuffer &buf, void *obj);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::RooJohnson*)
   {
      ::RooJohnson *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::RooJohnson >(0);
      static ::ROOT::TGenericClassInfo 
         instance("RooJohnson", ::RooJohnson::Class_Version(), "roofit/RooJohnson.h", 32,
                  typeid(::RooJohnson), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::RooJohnson::Dictionary, isa_proxy, 16,
                  sizeof(::RooJohnson) );
      instance.SetNew(&new_RooJohnson);
      instance.SetNewArray(&newArray_RooJohnson);
      instance.SetDelete(&delete_RooJohnson);
      instance.SetDeleteArray(&deleteArray_RooJohnson);
      instance.SetDestructor(&destruct_RooJohnson);
      instance.SetStreamerFunc(&streamer_RooJohnson);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::RooJohnson*)
   {
      return GenerateInitInstanceLocal((::RooJohnson*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_(Init) = GenerateInitInstanceLocal((const ::RooJohnson*)0x0); R__UseDummy(_R__UNIQUE_(Init));
} // end of namespace ROOT

//______________________________________________________________________________
atomic_TClass_ptr RooJohnson::fgIsA(0);  // static to hold class pointer

//______________________________________________________________________________
const char *RooJohnson::Class_Name()
{
   return "RooJohnson";
}

//______________________________________________________________________________
const char *RooJohnson::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::RooJohnson*)0x0)->GetImplFileName();
}

//______________________________________________________________________________
int RooJohnson::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::RooJohnson*)0x0)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *RooJohnson::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::RooJohnson*)0x0)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *RooJohnson::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD2(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::RooJohnson*)0x0)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
void RooJohnson::Streamer(TBuffer &R__b)
{
   // Stream an object of class RooJohnson.

   UInt_t R__s, R__c;
   if (R__b.IsReading()) {
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c); if (R__v) { }
      RooAbsPdf::Streamer(R__b);
      x.Streamer(R__b);
      mean.Streamer(R__b);
      width.Streamer(R__b);
      nu.Streamer(R__b);
      tau.Streamer(R__b);
      R__b.CheckByteCount(R__s, R__c, RooJohnson::IsA());
   } else {
      R__c = R__b.WriteVersion(RooJohnson::IsA(), kTRUE);
      RooAbsPdf::Streamer(R__b);
      x.Streamer(R__b);
      mean.Streamer(R__b);
      width.Streamer(R__b);
      nu.Streamer(R__b);
      tau.Streamer(R__b);
      R__b.SetByteCount(R__c, kTRUE);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_RooJohnson(void *p) {
      return  p ? new(p) ::RooJohnson : new ::RooJohnson;
   }
   static void *newArray_RooJohnson(Long_t nElements, void *p) {
      return p ? new(p) ::RooJohnson[nElements] : new ::RooJohnson[nElements];
   }
   // Wrapper around operator delete
   static void delete_RooJohnson(void *p) {
      delete ((::RooJohnson*)p);
   }
   static void deleteArray_RooJohnson(void *p) {
      delete [] ((::RooJohnson*)p);
   }
   static void destruct_RooJohnson(void *p) {
      typedef ::RooJohnson current_t;
      ((current_t*)p)->~current_t();
   }
   // Wrapper around a custom streamer member function.
   static void streamer_RooJohnson(TBuffer &buf, void *obj) {
      ((::RooJohnson*)obj)->::RooJohnson::Streamer(buf);
   }
} // end of namespace ROOT for class ::RooJohnson

namespace {
  void TriggerDictionaryInitialization_RooJohnson_Impl() {
    static const char* headers[] = {
"roofit/RooJohnson.cpp",
0
    };
    static const char* includePaths[] = {
"/afs/cern.ch/sw/lcg/releases/ROOT/6.08.02-99084/x86_64-slc6-gcc49-opt/include",
"/afs/cern.ch/user/p/pluca/repositories/tools-easyanalysis/",
0
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "RooJohnson dictionary forward declarations' payload"
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_Autoloading_Map;
class __attribute__((annotate(R"ATTRDUMP(Your description goes here...)ATTRDUMP"))) __attribute__((annotate("$clingAutoload$roofit/RooJohnson.cpp")))  RooJohnson;
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "RooJohnson dictionary payload"

#ifndef G__VECTOR_HAS_CLASS_ITERATOR
  #define G__VECTOR_HAS_CLASS_ITERATOR 1
#endif

#define _BACKWARD_BACKWARD_WARNING_H
#include "roofit/RooJohnson.cpp"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[]={
"RooJohnson", payloadCode, "@",
nullptr};

    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("RooJohnson",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_RooJohnson_Impl, {}, classesHeaders);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_RooJohnson_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_RooJohnson() {
  TriggerDictionaryInitialization_RooJohnson_Impl();
}
