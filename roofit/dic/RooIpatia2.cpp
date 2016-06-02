// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME dIafsdIcerndOchdIuserdIpdIplucadIrepositoriesdIlhcbbhamdItoolsdIroofitdIdicdIRooIpatia2

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
#include "/afs/cern.ch/user/p/pluca/repositories/lhcbbham/tools/roofit/RooIpatia2.cpp"

// Header files passed via #pragma extra_include

namespace ROOT {
   static void *new_RooIpatia2(void *p = 0);
   static void *newArray_RooIpatia2(Long_t size, void *p);
   static void delete_RooIpatia2(void *p);
   static void deleteArray_RooIpatia2(void *p);
   static void destruct_RooIpatia2(void *p);
   static void streamer_RooIpatia2(TBuffer &buf, void *obj);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::RooIpatia2*)
   {
      ::RooIpatia2 *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::RooIpatia2 >(0);
      static ::ROOT::TGenericClassInfo 
         instance("RooIpatia2", ::RooIpatia2::Class_Version(), "roofit/RooIpatia2.h", 16,
                  typeid(::RooIpatia2), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::RooIpatia2::Dictionary, isa_proxy, 16,
                  sizeof(::RooIpatia2) );
      instance.SetNew(&new_RooIpatia2);
      instance.SetNewArray(&newArray_RooIpatia2);
      instance.SetDelete(&delete_RooIpatia2);
      instance.SetDeleteArray(&deleteArray_RooIpatia2);
      instance.SetDestructor(&destruct_RooIpatia2);
      instance.SetStreamerFunc(&streamer_RooIpatia2);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::RooIpatia2*)
   {
      return GenerateInitInstanceLocal((::RooIpatia2*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_(Init) = GenerateInitInstanceLocal((const ::RooIpatia2*)0x0); R__UseDummy(_R__UNIQUE_(Init));
} // end of namespace ROOT

//______________________________________________________________________________
atomic_TClass_ptr RooIpatia2::fgIsA(0);  // static to hold class pointer

//______________________________________________________________________________
const char *RooIpatia2::Class_Name()
{
   return "RooIpatia2";
}

//______________________________________________________________________________
const char *RooIpatia2::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::RooIpatia2*)0x0)->GetImplFileName();
}

//______________________________________________________________________________
int RooIpatia2::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::RooIpatia2*)0x0)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *RooIpatia2::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::RooIpatia2*)0x0)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *RooIpatia2::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD2(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::RooIpatia2*)0x0)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
void RooIpatia2::Streamer(TBuffer &R__b)
{
   // Stream an object of class RooIpatia2.

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
      a2.Streamer(R__b);
      n2.Streamer(R__b);
      R__b.CheckByteCount(R__s, R__c, RooIpatia2::IsA());
   } else {
      R__c = R__b.WriteVersion(RooIpatia2::IsA(), kTRUE);
      RooAbsPdf::Streamer(R__b);
      x.Streamer(R__b);
      l.Streamer(R__b);
      zeta.Streamer(R__b);
      fb.Streamer(R__b);
      sigma.Streamer(R__b);
      mu.Streamer(R__b);
      a.Streamer(R__b);
      n.Streamer(R__b);
      a2.Streamer(R__b);
      n2.Streamer(R__b);
      R__b.SetByteCount(R__c, kTRUE);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_RooIpatia2(void *p) {
      return  p ? new(p) ::RooIpatia2 : new ::RooIpatia2;
   }
   static void *newArray_RooIpatia2(Long_t nElements, void *p) {
      return p ? new(p) ::RooIpatia2[nElements] : new ::RooIpatia2[nElements];
   }
   // Wrapper around operator delete
   static void delete_RooIpatia2(void *p) {
      delete ((::RooIpatia2*)p);
   }
   static void deleteArray_RooIpatia2(void *p) {
      delete [] ((::RooIpatia2*)p);
   }
   static void destruct_RooIpatia2(void *p) {
      typedef ::RooIpatia2 current_t;
      ((current_t*)p)->~current_t();
   }
   // Wrapper around a custom streamer member function.
   static void streamer_RooIpatia2(TBuffer &buf, void *obj) {
      ((::RooIpatia2*)obj)->::RooIpatia2::Streamer(buf);
   }
} // end of namespace ROOT for class ::RooIpatia2

namespace {
  void TriggerDictionaryInitialization_RooIpatia2_Impl() {
    static const char* headers[] = {
"roofit/RooIpatia2.cpp",
0
    };
    static const char* includePaths[] = {
"/afs/cern.ch/sw/lcg/releases/ROOT/6.06.02-6cc9c/x86_64-slc6-gcc49-opt/include",
"/afs/cern.ch/user/p/pluca/repositories/lhcbbham/tools/",
0
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "RooIpatia2 dictionary forward declarations' payload"
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_Autoloading_Map;
class __attribute__((annotate(R"ATTRDUMP(Your description goes here...)ATTRDUMP"))) __attribute__((annotate("$clingAutoload$roofit/RooIpatia2.cpp")))  RooIpatia2;
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "RooIpatia2 dictionary payload"

#ifndef G__VECTOR_HAS_CLASS_ITERATOR
  #define G__VECTOR_HAS_CLASS_ITERATOR 1
#endif

#define _BACKWARD_BACKWARD_WARNING_H
#include "roofit/RooIpatia2.cpp"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[]={
"RooIpatia2", payloadCode, "@",
nullptr};

    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("RooIpatia2",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_RooIpatia2_Impl, {}, classesHeaders);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_RooIpatia2_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_RooIpatia2() {
  TriggerDictionaryInitialization_RooIpatia2_Impl();
}
