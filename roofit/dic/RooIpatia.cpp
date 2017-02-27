// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME dIafsdIcerndOchdIuserdIpdIplucadIrepositoriesdItoolsmIeasyanalysisdIroofitdIdicdIRooIpatia

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
#include "/afs/cern.ch/user/p/pluca/repositories/tools-easyanalysis/roofit/RooIpatia.cpp"

// Header files passed via #pragma extra_include

namespace ROOT {
   static void *new_RooIpatia(void *p = 0);
   static void *newArray_RooIpatia(Long_t size, void *p);
   static void delete_RooIpatia(void *p);
   static void deleteArray_RooIpatia(void *p);
   static void destruct_RooIpatia(void *p);
   static void streamer_RooIpatia(TBuffer &buf, void *obj);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::RooIpatia*)
   {
      ::RooIpatia *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::RooIpatia >(0);
      static ::ROOT::TGenericClassInfo 
         instance("RooIpatia", ::RooIpatia::Class_Version(), "roofit/RooIpatia.h", 16,
                  typeid(::RooIpatia), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::RooIpatia::Dictionary, isa_proxy, 16,
                  sizeof(::RooIpatia) );
      instance.SetNew(&new_RooIpatia);
      instance.SetNewArray(&newArray_RooIpatia);
      instance.SetDelete(&delete_RooIpatia);
      instance.SetDeleteArray(&deleteArray_RooIpatia);
      instance.SetDestructor(&destruct_RooIpatia);
      instance.SetStreamerFunc(&streamer_RooIpatia);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::RooIpatia*)
   {
      return GenerateInitInstanceLocal((::RooIpatia*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_(Init) = GenerateInitInstanceLocal((const ::RooIpatia*)0x0); R__UseDummy(_R__UNIQUE_(Init));
} // end of namespace ROOT

//______________________________________________________________________________
atomic_TClass_ptr RooIpatia::fgIsA(0);  // static to hold class pointer

//______________________________________________________________________________
const char *RooIpatia::Class_Name()
{
   return "RooIpatia";
}

//______________________________________________________________________________
const char *RooIpatia::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::RooIpatia*)0x0)->GetImplFileName();
}

//______________________________________________________________________________
int RooIpatia::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::RooIpatia*)0x0)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *RooIpatia::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::RooIpatia*)0x0)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *RooIpatia::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD2(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::RooIpatia*)0x0)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
void RooIpatia::Streamer(TBuffer &R__b)
{
   // Stream an object of class RooIpatia.

   UInt_t R__s, R__c;
   if (R__b.IsReading()) {
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c); if (R__v) { }
      RooAbsPdf::Streamer(R__b);
      x.Streamer(R__b);
      l.Streamer(R__b);
      zeta.Streamer(R__b);
      fb.Streamer(R__b);
      sigma.Streamer(R__b);
      mu.Streamer(R__b);
      a.Streamer(R__b);
      n.Streamer(R__b);
      R__b.CheckByteCount(R__s, R__c, RooIpatia::IsA());
   } else {
      R__c = R__b.WriteVersion(RooIpatia::IsA(), kTRUE);
      RooAbsPdf::Streamer(R__b);
      x.Streamer(R__b);
      l.Streamer(R__b);
      zeta.Streamer(R__b);
      fb.Streamer(R__b);
      sigma.Streamer(R__b);
      mu.Streamer(R__b);
      a.Streamer(R__b);
      n.Streamer(R__b);
      R__b.SetByteCount(R__c, kTRUE);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_RooIpatia(void *p) {
      return  p ? new(p) ::RooIpatia : new ::RooIpatia;
   }
   static void *newArray_RooIpatia(Long_t nElements, void *p) {
      return p ? new(p) ::RooIpatia[nElements] : new ::RooIpatia[nElements];
   }
   // Wrapper around operator delete
   static void delete_RooIpatia(void *p) {
      delete ((::RooIpatia*)p);
   }
   static void deleteArray_RooIpatia(void *p) {
      delete [] ((::RooIpatia*)p);
   }
   static void destruct_RooIpatia(void *p) {
      typedef ::RooIpatia current_t;
      ((current_t*)p)->~current_t();
   }
   // Wrapper around a custom streamer member function.
   static void streamer_RooIpatia(TBuffer &buf, void *obj) {
      ((::RooIpatia*)obj)->::RooIpatia::Streamer(buf);
   }
} // end of namespace ROOT for class ::RooIpatia

namespace {
  void TriggerDictionaryInitialization_RooIpatia_Impl() {
    static const char* headers[] = {
"roofit/RooIpatia.cpp",
0
    };
    static const char* includePaths[] = {
"/afs/cern.ch/sw/lcg/releases/ROOT/6.08.02-99084/x86_64-slc6-gcc49-opt/include",
"/afs/cern.ch/user/p/pluca/repositories/tools-easyanalysis/",
0
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "RooIpatia dictionary forward declarations' payload"
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_Autoloading_Map;
class __attribute__((annotate(R"ATTRDUMP(Your description goes here...)ATTRDUMP"))) __attribute__((annotate("$clingAutoload$roofit/RooIpatia.cpp")))  RooIpatia;
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "RooIpatia dictionary payload"

#ifndef G__VECTOR_HAS_CLASS_ITERATOR
  #define G__VECTOR_HAS_CLASS_ITERATOR 1
#endif

#define _BACKWARD_BACKWARD_WARNING_H
#include "roofit/RooIpatia.cpp"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[]={
"RooIpatia", payloadCode, "@",
nullptr};

    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("RooIpatia",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_RooIpatia_Impl, {}, classesHeaders);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_RooIpatia_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_RooIpatia() {
  TriggerDictionaryInitialization_RooIpatia_Impl();
}
