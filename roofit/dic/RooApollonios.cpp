// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME dIafsdIcerndOchdIuserdIpdIplucadIrepositoriesdIlhcbbhamdItoolsdIroofitdIdicdIRooApollonios

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
#include "/afs/cern.ch/user/p/pluca/repositories/lhcbbham/tools/roofit/RooApollonios.cpp"

// Header files passed via #pragma extra_include

namespace ROOT {
   static void *new_RooApollonios(void *p = 0);
   static void *newArray_RooApollonios(Long_t size, void *p);
   static void delete_RooApollonios(void *p);
   static void deleteArray_RooApollonios(void *p);
   static void destruct_RooApollonios(void *p);
   static void streamer_RooApollonios(TBuffer &buf, void *obj);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::RooApollonios*)
   {
      ::RooApollonios *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::RooApollonios >(0);
      static ::ROOT::TGenericClassInfo 
         instance("RooApollonios", ::RooApollonios::Class_Version(), "roofit/RooApollonios.h", 16,
                  typeid(::RooApollonios), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::RooApollonios::Dictionary, isa_proxy, 16,
                  sizeof(::RooApollonios) );
      instance.SetNew(&new_RooApollonios);
      instance.SetNewArray(&newArray_RooApollonios);
      instance.SetDelete(&delete_RooApollonios);
      instance.SetDeleteArray(&deleteArray_RooApollonios);
      instance.SetDestructor(&destruct_RooApollonios);
      instance.SetStreamerFunc(&streamer_RooApollonios);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::RooApollonios*)
   {
      return GenerateInitInstanceLocal((::RooApollonios*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_(Init) = GenerateInitInstanceLocal((const ::RooApollonios*)0x0); R__UseDummy(_R__UNIQUE_(Init));
} // end of namespace ROOT

//______________________________________________________________________________
atomic_TClass_ptr RooApollonios::fgIsA(0);  // static to hold class pointer

//______________________________________________________________________________
const char *RooApollonios::Class_Name()
{
   return "RooApollonios";
}

//______________________________________________________________________________
const char *RooApollonios::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::RooApollonios*)0x0)->GetImplFileName();
}

//______________________________________________________________________________
int RooApollonios::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::RooApollonios*)0x0)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *RooApollonios::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::RooApollonios*)0x0)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *RooApollonios::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD2(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::RooApollonios*)0x0)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
void RooApollonios::Streamer(TBuffer &R__b)
{
   // Stream an object of class RooApollonios.

   UInt_t R__s, R__c;
   if (R__b.IsReading()) {
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c); if (R__v) { }
      RooAbsPdf::Streamer(R__b);
      m.Streamer(R__b);
      m0.Streamer(R__b);
      sigma.Streamer(R__b);
      b.Streamer(R__b);
      a.Streamer(R__b);
      n.Streamer(R__b);
      R__b.CheckByteCount(R__s, R__c, RooApollonios::IsA());
   } else {
      R__c = R__b.WriteVersion(RooApollonios::IsA(), kTRUE);
      RooAbsPdf::Streamer(R__b);
      m.Streamer(R__b);
      m0.Streamer(R__b);
      sigma.Streamer(R__b);
      b.Streamer(R__b);
      a.Streamer(R__b);
      n.Streamer(R__b);
      R__b.SetByteCount(R__c, kTRUE);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_RooApollonios(void *p) {
      return  p ? new(p) ::RooApollonios : new ::RooApollonios;
   }
   static void *newArray_RooApollonios(Long_t nElements, void *p) {
      return p ? new(p) ::RooApollonios[nElements] : new ::RooApollonios[nElements];
   }
   // Wrapper around operator delete
   static void delete_RooApollonios(void *p) {
      delete ((::RooApollonios*)p);
   }
   static void deleteArray_RooApollonios(void *p) {
      delete [] ((::RooApollonios*)p);
   }
   static void destruct_RooApollonios(void *p) {
      typedef ::RooApollonios current_t;
      ((current_t*)p)->~current_t();
   }
   // Wrapper around a custom streamer member function.
   static void streamer_RooApollonios(TBuffer &buf, void *obj) {
      ((::RooApollonios*)obj)->::RooApollonios::Streamer(buf);
   }
} // end of namespace ROOT for class ::RooApollonios

namespace {
  void TriggerDictionaryInitialization_RooApollonios_Impl() {
    static const char* headers[] = {
"roofit/RooApollonios.cpp",
0
    };
    static const char* includePaths[] = {
"/afs/cern.ch/sw/lcg/releases/ROOT/6.06.02-6cc9c/x86_64-slc6-gcc49-opt/include",
"/afs/cern.ch/user/p/pluca/repositories/lhcbbham/tools/",
0
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "RooApollonios dictionary forward declarations' payload"
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_Autoloading_Map;
class __attribute__((annotate(R"ATTRDUMP(Your description goes here...)ATTRDUMP"))) __attribute__((annotate("$clingAutoload$roofit/RooApollonios.cpp")))  RooApollonios;
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "RooApollonios dictionary payload"

#ifndef G__VECTOR_HAS_CLASS_ITERATOR
  #define G__VECTOR_HAS_CLASS_ITERATOR 1
#endif

#define _BACKWARD_BACKWARD_WARNING_H
#include "roofit/RooApollonios.cpp"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[]={
"RooApollonios", payloadCode, "@",
nullptr};

    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("RooApollonios",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_RooApollonios_Impl, {}, classesHeaders);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_RooApollonios_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_RooApollonios() {
  TriggerDictionaryInitialization_RooApollonios_Impl();
}
