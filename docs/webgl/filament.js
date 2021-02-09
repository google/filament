
var Filament = (function() {
  var _scriptDir = typeof document !== 'undefined' && document.currentScript ? document.currentScript.src : undefined;
  if (typeof __filename !== 'undefined') _scriptDir = _scriptDir || __filename;
  return (
function(Filament) {
  Filament = Filament || {};

var Module=typeof Filament!=="undefined"?Filament:{};var readyPromiseResolve,readyPromiseReject;Module["ready"]=new Promise(function(resolve,reject){readyPromiseResolve=resolve;readyPromiseReject=reject});var moduleOverrides={};var key;for(key in Module){if(Module.hasOwnProperty(key)){moduleOverrides[key]=Module[key]}}var arguments_=[];var thisProgram="./this.program";var quit_=function(status,toThrow){throw toThrow};var ENVIRONMENT_IS_WEB=false;var ENVIRONMENT_IS_WORKER=false;var ENVIRONMENT_IS_NODE=false;var ENVIRONMENT_IS_SHELL=false;ENVIRONMENT_IS_WEB=typeof window==="object";ENVIRONMENT_IS_WORKER=typeof importScripts==="function";ENVIRONMENT_IS_NODE=typeof process==="object"&&typeof process.versions==="object"&&typeof process.versions.node==="string";ENVIRONMENT_IS_SHELL=!ENVIRONMENT_IS_WEB&&!ENVIRONMENT_IS_NODE&&!ENVIRONMENT_IS_WORKER;var scriptDirectory="";function locateFile(path){if(Module["locateFile"]){return Module["locateFile"](path,scriptDirectory)}return scriptDirectory+path}var read_,readAsync,readBinary,setWindowTitle;var nodeFS;var nodePath;if(ENVIRONMENT_IS_NODE){if(ENVIRONMENT_IS_WORKER){scriptDirectory=require("path").dirname(scriptDirectory)+"/"}else{scriptDirectory=__dirname+"/"}read_=function shell_read(filename,binary){if(!nodeFS)nodeFS=require("fs");if(!nodePath)nodePath=require("path");filename=nodePath["normalize"](filename);return nodeFS["readFileSync"](filename,binary?null:"utf8")};readBinary=function readBinary(filename){var ret=read_(filename,true);if(!ret.buffer){ret=new Uint8Array(ret)}assert(ret.buffer);return ret};if(process["argv"].length>1){thisProgram=process["argv"][1].replace(/\\/g,"/")}arguments_=process["argv"].slice(2);process["on"]("uncaughtException",function(ex){if(!(ex instanceof ExitStatus)){throw ex}});process["on"]("unhandledRejection",abort);quit_=function(status){process["exit"](status)};Module["inspect"]=function(){return"[Emscripten Module object]"}}else if(ENVIRONMENT_IS_SHELL){if(typeof read!="undefined"){read_=function shell_read(f){return read(f)}}readBinary=function readBinary(f){var data;if(typeof readbuffer==="function"){return new Uint8Array(readbuffer(f))}data=read(f,"binary");assert(typeof data==="object");return data};if(typeof scriptArgs!="undefined"){arguments_=scriptArgs}else if(typeof arguments!="undefined"){arguments_=arguments}if(typeof quit==="function"){quit_=function(status){quit(status)}}if(typeof print!=="undefined"){if(typeof console==="undefined")console={};console.log=print;console.warn=console.error=typeof printErr!=="undefined"?printErr:print}}else if(ENVIRONMENT_IS_WEB||ENVIRONMENT_IS_WORKER){if(ENVIRONMENT_IS_WORKER){scriptDirectory=self.location.href}else if(typeof document!=="undefined"&&document.currentScript){scriptDirectory=document.currentScript.src}if(_scriptDir){scriptDirectory=_scriptDir}if(scriptDirectory.indexOf("blob:")!==0){scriptDirectory=scriptDirectory.substr(0,scriptDirectory.lastIndexOf("/")+1)}else{scriptDirectory=""}{read_=function(url){var xhr=new XMLHttpRequest;xhr.open("GET",url,false);xhr.send(null);return xhr.responseText};if(ENVIRONMENT_IS_WORKER){readBinary=function(url){var xhr=new XMLHttpRequest;xhr.open("GET",url,false);xhr.responseType="arraybuffer";xhr.send(null);return new Uint8Array(xhr.response)}}readAsync=function(url,onload,onerror){var xhr=new XMLHttpRequest;xhr.open("GET",url,true);xhr.responseType="arraybuffer";xhr.onload=function(){if(xhr.status==200||xhr.status==0&&xhr.response){onload(xhr.response);return}onerror()};xhr.onerror=onerror;xhr.send(null)}}setWindowTitle=function(title){document.title=title}}else{}var out=Module["print"]||console.log.bind(console);var err=Module["printErr"]||console.warn.bind(console);for(key in moduleOverrides){if(moduleOverrides.hasOwnProperty(key)){Module[key]=moduleOverrides[key]}}moduleOverrides=null;if(Module["arguments"])arguments_=Module["arguments"];if(Module["thisProgram"])thisProgram=Module["thisProgram"];if(Module["quit"])quit_=Module["quit"];var tempRet0=0;var setTempRet0=function(value){tempRet0=value};var wasmBinary;if(Module["wasmBinary"])wasmBinary=Module["wasmBinary"];var noExitRuntime;if(Module["noExitRuntime"])noExitRuntime=Module["noExitRuntime"];if(typeof WebAssembly!=="object"){abort("no native wasm support detected")}var wasmMemory;var ABORT=false;var EXITSTATUS;function assert(condition,text){if(!condition){abort("Assertion failed: "+text)}}var UTF8Decoder=typeof TextDecoder!=="undefined"?new TextDecoder("utf8"):undefined;function UTF8ArrayToString(heap,idx,maxBytesToRead){var endIdx=idx+maxBytesToRead;var endPtr=idx;while(heap[endPtr]&&!(endPtr>=endIdx))++endPtr;if(endPtr-idx>16&&heap.subarray&&UTF8Decoder){return UTF8Decoder.decode(heap.subarray(idx,endPtr))}else{var str="";while(idx<endPtr){var u0=heap[idx++];if(!(u0&128)){str+=String.fromCharCode(u0);continue}var u1=heap[idx++]&63;if((u0&224)==192){str+=String.fromCharCode((u0&31)<<6|u1);continue}var u2=heap[idx++]&63;if((u0&240)==224){u0=(u0&15)<<12|u1<<6|u2}else{u0=(u0&7)<<18|u1<<12|u2<<6|heap[idx++]&63}if(u0<65536){str+=String.fromCharCode(u0)}else{var ch=u0-65536;str+=String.fromCharCode(55296|ch>>10,56320|ch&1023)}}}return str}function UTF8ToString(ptr,maxBytesToRead){return ptr?UTF8ArrayToString(HEAPU8,ptr,maxBytesToRead):""}function stringToUTF8Array(str,heap,outIdx,maxBytesToWrite){if(!(maxBytesToWrite>0))return 0;var startIdx=outIdx;var endIdx=outIdx+maxBytesToWrite-1;for(var i=0;i<str.length;++i){var u=str.charCodeAt(i);if(u>=55296&&u<=57343){var u1=str.charCodeAt(++i);u=65536+((u&1023)<<10)|u1&1023}if(u<=127){if(outIdx>=endIdx)break;heap[outIdx++]=u}else if(u<=2047){if(outIdx+1>=endIdx)break;heap[outIdx++]=192|u>>6;heap[outIdx++]=128|u&63}else if(u<=65535){if(outIdx+2>=endIdx)break;heap[outIdx++]=224|u>>12;heap[outIdx++]=128|u>>6&63;heap[outIdx++]=128|u&63}else{if(outIdx+3>=endIdx)break;heap[outIdx++]=240|u>>18;heap[outIdx++]=128|u>>12&63;heap[outIdx++]=128|u>>6&63;heap[outIdx++]=128|u&63}}heap[outIdx]=0;return outIdx-startIdx}function stringToUTF8(str,outPtr,maxBytesToWrite){return stringToUTF8Array(str,HEAPU8,outPtr,maxBytesToWrite)}function lengthBytesUTF8(str){var len=0;for(var i=0;i<str.length;++i){var u=str.charCodeAt(i);if(u>=55296&&u<=57343)u=65536+((u&1023)<<10)|str.charCodeAt(++i)&1023;if(u<=127)++len;else if(u<=2047)len+=2;else if(u<=65535)len+=3;else len+=4}return len}var UTF16Decoder=typeof TextDecoder!=="undefined"?new TextDecoder("utf-16le"):undefined;function UTF16ToString(ptr,maxBytesToRead){var endPtr=ptr;var idx=endPtr>>1;var maxIdx=idx+maxBytesToRead/2;while(!(idx>=maxIdx)&&HEAPU16[idx])++idx;endPtr=idx<<1;if(endPtr-ptr>32&&UTF16Decoder){return UTF16Decoder.decode(HEAPU8.subarray(ptr,endPtr))}else{var str="";for(var i=0;!(i>=maxBytesToRead/2);++i){var codeUnit=HEAP16[ptr+i*2>>1];if(codeUnit==0)break;str+=String.fromCharCode(codeUnit)}return str}}function stringToUTF16(str,outPtr,maxBytesToWrite){if(maxBytesToWrite===undefined){maxBytesToWrite=2147483647}if(maxBytesToWrite<2)return 0;maxBytesToWrite-=2;var startPtr=outPtr;var numCharsToWrite=maxBytesToWrite<str.length*2?maxBytesToWrite/2:str.length;for(var i=0;i<numCharsToWrite;++i){var codeUnit=str.charCodeAt(i);HEAP16[outPtr>>1]=codeUnit;outPtr+=2}HEAP16[outPtr>>1]=0;return outPtr-startPtr}function lengthBytesUTF16(str){return str.length*2}function UTF32ToString(ptr,maxBytesToRead){var i=0;var str="";while(!(i>=maxBytesToRead/4)){var utf32=HEAP32[ptr+i*4>>2];if(utf32==0)break;++i;if(utf32>=65536){var ch=utf32-65536;str+=String.fromCharCode(55296|ch>>10,56320|ch&1023)}else{str+=String.fromCharCode(utf32)}}return str}function stringToUTF32(str,outPtr,maxBytesToWrite){if(maxBytesToWrite===undefined){maxBytesToWrite=2147483647}if(maxBytesToWrite<4)return 0;var startPtr=outPtr;var endPtr=startPtr+maxBytesToWrite-4;for(var i=0;i<str.length;++i){var codeUnit=str.charCodeAt(i);if(codeUnit>=55296&&codeUnit<=57343){var trailSurrogate=str.charCodeAt(++i);codeUnit=65536+((codeUnit&1023)<<10)|trailSurrogate&1023}HEAP32[outPtr>>2]=codeUnit;outPtr+=4;if(outPtr+4>endPtr)break}HEAP32[outPtr>>2]=0;return outPtr-startPtr}function lengthBytesUTF32(str){var len=0;for(var i=0;i<str.length;++i){var codeUnit=str.charCodeAt(i);if(codeUnit>=55296&&codeUnit<=57343)++i;len+=4}return len}function alignUp(x,multiple){if(x%multiple>0){x+=multiple-x%multiple}return x}var buffer,HEAP8,HEAPU8,HEAP16,HEAPU16,HEAP32,HEAPU32,HEAPF32,HEAPF64;function updateGlobalBufferAndViews(buf){buffer=buf;Module["HEAP8"]=HEAP8=new Int8Array(buf);Module["HEAP16"]=HEAP16=new Int16Array(buf);Module["HEAP32"]=HEAP32=new Int32Array(buf);Module["HEAPU8"]=HEAPU8=new Uint8Array(buf);Module["HEAPU16"]=HEAPU16=new Uint16Array(buf);Module["HEAPU32"]=HEAPU32=new Uint32Array(buf);Module["HEAPF32"]=HEAPF32=new Float32Array(buf);Module["HEAPF64"]=HEAPF64=new Float64Array(buf)}var INITIAL_MEMORY=Module["INITIAL_MEMORY"]||16777216;var wasmTable;var __ATPRERUN__=[];var __ATINIT__=[];var __ATMAIN__=[];var __ATPOSTRUN__=[];var runtimeInitialized=false;__ATINIT__.push({func:function(){___wasm_call_ctors()}});function preRun(){if(Module["preRun"]){if(typeof Module["preRun"]=="function")Module["preRun"]=[Module["preRun"]];while(Module["preRun"].length){addOnPreRun(Module["preRun"].shift())}}callRuntimeCallbacks(__ATPRERUN__)}function initRuntime(){runtimeInitialized=true;callRuntimeCallbacks(__ATINIT__)}function preMain(){callRuntimeCallbacks(__ATMAIN__)}function postRun(){if(Module["postRun"]){if(typeof Module["postRun"]=="function")Module["postRun"]=[Module["postRun"]];while(Module["postRun"].length){addOnPostRun(Module["postRun"].shift())}}callRuntimeCallbacks(__ATPOSTRUN__)}function addOnPreRun(cb){__ATPRERUN__.unshift(cb)}function addOnPostRun(cb){__ATPOSTRUN__.unshift(cb)}var runDependencies=0;var runDependencyWatcher=null;var dependenciesFulfilled=null;function addRunDependency(id){runDependencies++;if(Module["monitorRunDependencies"]){Module["monitorRunDependencies"](runDependencies)}}function removeRunDependency(id){runDependencies--;if(Module["monitorRunDependencies"]){Module["monitorRunDependencies"](runDependencies)}if(runDependencies==0){if(runDependencyWatcher!==null){clearInterval(runDependencyWatcher);runDependencyWatcher=null}if(dependenciesFulfilled){var callback=dependenciesFulfilled;dependenciesFulfilled=null;callback()}}}Module["preloadedImages"]={};Module["preloadedAudios"]={};function abort(what){if(Module["onAbort"]){Module["onAbort"](what)}what+="";err(what);ABORT=true;EXITSTATUS=1;what="abort("+what+"). Build with -s ASSERTIONS=1 for more info.";var e=new WebAssembly.RuntimeError(what);readyPromiseReject(e);throw e}function hasPrefix(str,prefix){return String.prototype.startsWith?str.startsWith(prefix):str.indexOf(prefix)===0}var dataURIPrefix="data:application/octet-stream;base64,";function isDataURI(filename){return hasPrefix(filename,dataURIPrefix)}var fileURIPrefix="file://";function isFileURI(filename){return hasPrefix(filename,fileURIPrefix)}var wasmBinaryFile="filament.wasm";if(!isDataURI(wasmBinaryFile)){wasmBinaryFile=locateFile(wasmBinaryFile)}function getBinary(file){try{if(file==wasmBinaryFile&&wasmBinary){return new Uint8Array(wasmBinary)}if(readBinary){return readBinary(file)}else{throw"both async and sync fetching of the wasm failed"}}catch(err){abort(err)}}function getBinaryPromise(){if(!wasmBinary&&(ENVIRONMENT_IS_WEB||ENVIRONMENT_IS_WORKER)){if(typeof fetch==="function"&&!isFileURI(wasmBinaryFile)){return fetch(wasmBinaryFile,{credentials:"same-origin"}).then(function(response){if(!response["ok"]){throw"failed to load wasm binary file at '"+wasmBinaryFile+"'"}return response["arrayBuffer"]()}).catch(function(){return getBinary(wasmBinaryFile)})}else{if(readAsync){return new Promise(function(resolve,reject){readAsync(wasmBinaryFile,function(response){resolve(new Uint8Array(response))},reject)})}}}return Promise.resolve().then(function(){return getBinary(wasmBinaryFile)})}function createWasm(){var info={"a":asmLibraryArg};function receiveInstance(instance,module){var exports=instance.exports;Module["asm"]=exports;wasmMemory=Module["asm"]["Sg"];updateGlobalBufferAndViews(wasmMemory.buffer);wasmTable=Module["asm"]["Tg"];removeRunDependency("wasm-instantiate")}addRunDependency("wasm-instantiate");function receiveInstantiatedSource(output){receiveInstance(output["instance"])}function instantiateArrayBuffer(receiver){return getBinaryPromise().then(function(binary){return WebAssembly.instantiate(binary,info)}).then(receiver,function(reason){err("failed to asynchronously prepare wasm: "+reason);abort(reason)})}function instantiateAsync(){if(!wasmBinary&&typeof WebAssembly.instantiateStreaming==="function"&&!isDataURI(wasmBinaryFile)&&!isFileURI(wasmBinaryFile)&&typeof fetch==="function"){return fetch(wasmBinaryFile,{credentials:"same-origin"}).then(function(response){var result=WebAssembly.instantiateStreaming(response,info);return result.then(receiveInstantiatedSource,function(reason){err("wasm streaming compile failed: "+reason);err("falling back to ArrayBuffer instantiation");return instantiateArrayBuffer(receiveInstantiatedSource)})})}else{return instantiateArrayBuffer(receiveInstantiatedSource)}}if(Module["instantiateWasm"]){try{var exports=Module["instantiateWasm"](info,receiveInstance);return exports}catch(e){err("Module.instantiateWasm callback failed with error: "+e);return false}}instantiateAsync().catch(readyPromiseReject);return{}}var ASM_CONSTS={9985:function(){const handle=GL.registerContext(Filament.glContext,Filament.glOptions);GL.makeContextCurrent(handle)},197660:function(){GL.currArrayBuffer=GLctx.currentArrayBufferBinding}};function callRuntimeCallbacks(callbacks){while(callbacks.length>0){var callback=callbacks.shift();if(typeof callback=="function"){callback(Module);continue}var func=callback.func;if(typeof func==="number"){if(callback.arg===undefined){wasmTable.get(func)()}else{wasmTable.get(func)(callback.arg)}}else{func(callback.arg===undefined?null:callback.arg)}}}var tupleRegistrations={};function runDestructors(destructors){while(destructors.length){var ptr=destructors.pop();var del=destructors.pop();del(ptr)}}function simpleReadValueFromPointer(pointer){return this["fromWireType"](HEAPU32[pointer>>2])}var awaitingDependencies={};var registeredTypes={};var typeDependencies={};var char_0=48;var char_9=57;function makeLegalFunctionName(name){if(undefined===name){return"_unknown"}name=name.replace(/[^a-zA-Z0-9_]/g,"$");var f=name.charCodeAt(0);if(f>=char_0&&f<=char_9){return"_"+name}else{return name}}function createNamedFunction(name,body){name=makeLegalFunctionName(name);return new Function("body","return function "+name+"() {\n"+'    "use strict";'+"    return body.apply(this, arguments);\n"+"};\n")(body)}function extendError(baseErrorType,errorName){var errorClass=createNamedFunction(errorName,function(message){this.name=errorName;this.message=message;var stack=new Error(message).stack;if(stack!==undefined){this.stack=this.toString()+"\n"+stack.replace(/^Error(:[^\n]*)?\n/,"")}});errorClass.prototype=Object.create(baseErrorType.prototype);errorClass.prototype.constructor=errorClass;errorClass.prototype.toString=function(){if(this.message===undefined){return this.name}else{return this.name+": "+this.message}};return errorClass}var InternalError=undefined;function throwInternalError(message){throw new InternalError(message)}function whenDependentTypesAreResolved(myTypes,dependentTypes,getTypeConverters){myTypes.forEach(function(type){typeDependencies[type]=dependentTypes});function onComplete(typeConverters){var myTypeConverters=getTypeConverters(typeConverters);if(myTypeConverters.length!==myTypes.length){throwInternalError("Mismatched type converter count")}for(var i=0;i<myTypes.length;++i){registerType(myTypes[i],myTypeConverters[i])}}var typeConverters=new Array(dependentTypes.length);var unregisteredTypes=[];var registered=0;dependentTypes.forEach(function(dt,i){if(registeredTypes.hasOwnProperty(dt)){typeConverters[i]=registeredTypes[dt]}else{unregisteredTypes.push(dt);if(!awaitingDependencies.hasOwnProperty(dt)){awaitingDependencies[dt]=[]}awaitingDependencies[dt].push(function(){typeConverters[i]=registeredTypes[dt];++registered;if(registered===unregisteredTypes.length){onComplete(typeConverters)}})}});if(0===unregisteredTypes.length){onComplete(typeConverters)}}function __embind_finalize_value_array(rawTupleType){var reg=tupleRegistrations[rawTupleType];delete tupleRegistrations[rawTupleType];var elements=reg.elements;var elementsLength=elements.length;var elementTypes=elements.map(function(elt){return elt.getterReturnType}).concat(elements.map(function(elt){return elt.setterArgumentType}));var rawConstructor=reg.rawConstructor;var rawDestructor=reg.rawDestructor;whenDependentTypesAreResolved([rawTupleType],elementTypes,function(elementTypes){elements.forEach(function(elt,i){var getterReturnType=elementTypes[i];var getter=elt.getter;var getterContext=elt.getterContext;var setterArgumentType=elementTypes[i+elementsLength];var setter=elt.setter;var setterContext=elt.setterContext;elt.read=function(ptr){return getterReturnType["fromWireType"](getter(getterContext,ptr))};elt.write=function(ptr,o){var destructors=[];setter(setterContext,ptr,setterArgumentType["toWireType"](destructors,o));runDestructors(destructors)}});return[{name:reg.name,"fromWireType":function(ptr){var rv=new Array(elementsLength);for(var i=0;i<elementsLength;++i){rv[i]=elements[i].read(ptr)}rawDestructor(ptr);return rv},"toWireType":function(destructors,o){if(elementsLength!==o.length){throw new TypeError("Incorrect number of tuple elements for "+reg.name+": expected="+elementsLength+", actual="+o.length)}var ptr=rawConstructor();for(var i=0;i<elementsLength;++i){elements[i].write(ptr,o[i])}if(destructors!==null){destructors.push(rawDestructor,ptr)}return ptr},"argPackAdvance":8,"readValueFromPointer":simpleReadValueFromPointer,destructorFunction:rawDestructor}]})}var structRegistrations={};function __embind_finalize_value_object(structType){var reg=structRegistrations[structType];delete structRegistrations[structType];var rawConstructor=reg.rawConstructor;var rawDestructor=reg.rawDestructor;var fieldRecords=reg.fields;var fieldTypes=fieldRecords.map(function(field){return field.getterReturnType}).concat(fieldRecords.map(function(field){return field.setterArgumentType}));whenDependentTypesAreResolved([structType],fieldTypes,function(fieldTypes){var fields={};fieldRecords.forEach(function(field,i){var fieldName=field.fieldName;var getterReturnType=fieldTypes[i];var getter=field.getter;var getterContext=field.getterContext;var setterArgumentType=fieldTypes[i+fieldRecords.length];var setter=field.setter;var setterContext=field.setterContext;fields[fieldName]={read:function(ptr){return getterReturnType["fromWireType"](getter(getterContext,ptr))},write:function(ptr,o){var destructors=[];setter(setterContext,ptr,setterArgumentType["toWireType"](destructors,o));runDestructors(destructors)}}});return[{name:reg.name,"fromWireType":function(ptr){var rv={};for(var i in fields){rv[i]=fields[i].read(ptr)}rawDestructor(ptr);return rv},"toWireType":function(destructors,o){for(var fieldName in fields){if(!(fieldName in o)){throw new TypeError('Missing field:  "'+fieldName+'"')}}var ptr=rawConstructor();for(fieldName in fields){fields[fieldName].write(ptr,o[fieldName])}if(destructors!==null){destructors.push(rawDestructor,ptr)}return ptr},"argPackAdvance":8,"readValueFromPointer":simpleReadValueFromPointer,destructorFunction:rawDestructor}]})}function getShiftFromSize(size){switch(size){case 1:return 0;case 2:return 1;case 4:return 2;case 8:return 3;default:throw new TypeError("Unknown type size: "+size)}}function embind_init_charCodes(){var codes=new Array(256);for(var i=0;i<256;++i){codes[i]=String.fromCharCode(i)}embind_charCodes=codes}var embind_charCodes=undefined;function readLatin1String(ptr){var ret="";var c=ptr;while(HEAPU8[c]){ret+=embind_charCodes[HEAPU8[c++]]}return ret}var BindingError=undefined;function throwBindingError(message){throw new BindingError(message)}function registerType(rawType,registeredInstance,options){options=options||{};if(!("argPackAdvance"in registeredInstance)){throw new TypeError("registerType registeredInstance requires argPackAdvance")}var name=registeredInstance.name;if(!rawType){throwBindingError('type "'+name+'" must have a positive integer typeid pointer')}if(registeredTypes.hasOwnProperty(rawType)){if(options.ignoreDuplicateRegistrations){return}else{throwBindingError("Cannot register type '"+name+"' twice")}}registeredTypes[rawType]=registeredInstance;delete typeDependencies[rawType];if(awaitingDependencies.hasOwnProperty(rawType)){var callbacks=awaitingDependencies[rawType];delete awaitingDependencies[rawType];callbacks.forEach(function(cb){cb()})}}function __embind_register_bool(rawType,name,size,trueValue,falseValue){var shift=getShiftFromSize(size);name=readLatin1String(name);registerType(rawType,{name:name,"fromWireType":function(wt){return!!wt},"toWireType":function(destructors,o){return o?trueValue:falseValue},"argPackAdvance":8,"readValueFromPointer":function(pointer){var heap;if(size===1){heap=HEAP8}else if(size===2){heap=HEAP16}else if(size===4){heap=HEAP32}else{throw new TypeError("Unknown boolean type size: "+name)}return this["fromWireType"](heap[pointer>>shift])},destructorFunction:null})}function ClassHandle_isAliasOf(other){if(!(this instanceof ClassHandle)){return false}if(!(other instanceof ClassHandle)){return false}var leftClass=this.$$.ptrType.registeredClass;var left=this.$$.ptr;var rightClass=other.$$.ptrType.registeredClass;var right=other.$$.ptr;while(leftClass.baseClass){left=leftClass.upcast(left);leftClass=leftClass.baseClass}while(rightClass.baseClass){right=rightClass.upcast(right);rightClass=rightClass.baseClass}return leftClass===rightClass&&left===right}function shallowCopyInternalPointer(o){return{count:o.count,deleteScheduled:o.deleteScheduled,preservePointerOnDelete:o.preservePointerOnDelete,ptr:o.ptr,ptrType:o.ptrType,smartPtr:o.smartPtr,smartPtrType:o.smartPtrType}}function throwInstanceAlreadyDeleted(obj){function getInstanceTypeName(handle){return handle.$$.ptrType.registeredClass.name}throwBindingError(getInstanceTypeName(obj)+" instance already deleted")}var finalizationGroup=false;function detachFinalizer(handle){}function runDestructor($$){if($$.smartPtr){$$.smartPtrType.rawDestructor($$.smartPtr)}else{$$.ptrType.registeredClass.rawDestructor($$.ptr)}}function releaseClassHandle($$){$$.count.value-=1;var toDelete=0===$$.count.value;if(toDelete){runDestructor($$)}}function attachFinalizer(handle){if("undefined"===typeof FinalizationGroup){attachFinalizer=function(handle){return handle};return handle}finalizationGroup=new FinalizationGroup(function(iter){for(var result=iter.next();!result.done;result=iter.next()){var $$=result.value;if(!$$.ptr){console.warn("object already deleted: "+$$.ptr)}else{releaseClassHandle($$)}}});attachFinalizer=function(handle){finalizationGroup.register(handle,handle.$$,handle.$$);return handle};detachFinalizer=function(handle){finalizationGroup.unregister(handle.$$)};return attachFinalizer(handle)}function ClassHandle_clone(){if(!this.$$.ptr){throwInstanceAlreadyDeleted(this)}if(this.$$.preservePointerOnDelete){this.$$.count.value+=1;return this}else{var clone=attachFinalizer(Object.create(Object.getPrototypeOf(this),{$$:{value:shallowCopyInternalPointer(this.$$)}}));clone.$$.count.value+=1;clone.$$.deleteScheduled=false;return clone}}function ClassHandle_delete(){if(!this.$$.ptr){throwInstanceAlreadyDeleted(this)}if(this.$$.deleteScheduled&&!this.$$.preservePointerOnDelete){throwBindingError("Object already scheduled for deletion")}detachFinalizer(this);releaseClassHandle(this.$$);if(!this.$$.preservePointerOnDelete){this.$$.smartPtr=undefined;this.$$.ptr=undefined}}function ClassHandle_isDeleted(){return!this.$$.ptr}var delayFunction=undefined;var deletionQueue=[];function flushPendingDeletes(){while(deletionQueue.length){var obj=deletionQueue.pop();obj.$$.deleteScheduled=false;obj["delete"]()}}function ClassHandle_deleteLater(){if(!this.$$.ptr){throwInstanceAlreadyDeleted(this)}if(this.$$.deleteScheduled&&!this.$$.preservePointerOnDelete){throwBindingError("Object already scheduled for deletion")}deletionQueue.push(this);if(deletionQueue.length===1&&delayFunction){delayFunction(flushPendingDeletes)}this.$$.deleteScheduled=true;return this}function init_ClassHandle(){ClassHandle.prototype["isAliasOf"]=ClassHandle_isAliasOf;ClassHandle.prototype["clone"]=ClassHandle_clone;ClassHandle.prototype["delete"]=ClassHandle_delete;ClassHandle.prototype["isDeleted"]=ClassHandle_isDeleted;ClassHandle.prototype["deleteLater"]=ClassHandle_deleteLater}function ClassHandle(){}var registeredPointers={};function ensureOverloadTable(proto,methodName,humanName){if(undefined===proto[methodName].overloadTable){var prevFunc=proto[methodName];proto[methodName]=function(){if(!proto[methodName].overloadTable.hasOwnProperty(arguments.length)){throwBindingError("Function '"+humanName+"' called with an invalid number of arguments ("+arguments.length+") - expects one of ("+proto[methodName].overloadTable+")!")}return proto[methodName].overloadTable[arguments.length].apply(this,arguments)};proto[methodName].overloadTable=[];proto[methodName].overloadTable[prevFunc.argCount]=prevFunc}}function exposePublicSymbol(name,value,numArguments){if(Module.hasOwnProperty(name)){if(undefined===numArguments||undefined!==Module[name].overloadTable&&undefined!==Module[name].overloadTable[numArguments]){throwBindingError("Cannot register public name '"+name+"' twice")}ensureOverloadTable(Module,name,name);if(Module.hasOwnProperty(numArguments)){throwBindingError("Cannot register multiple overloads of a function with the same number of arguments ("+numArguments+")!")}Module[name].overloadTable[numArguments]=value}else{Module[name]=value;if(undefined!==numArguments){Module[name].numArguments=numArguments}}}function RegisteredClass(name,constructor,instancePrototype,rawDestructor,baseClass,getActualType,upcast,downcast){this.name=name;this.constructor=constructor;this.instancePrototype=instancePrototype;this.rawDestructor=rawDestructor;this.baseClass=baseClass;this.getActualType=getActualType;this.upcast=upcast;this.downcast=downcast;this.pureVirtualFunctions=[]}function upcastPointer(ptr,ptrClass,desiredClass){while(ptrClass!==desiredClass){if(!ptrClass.upcast){throwBindingError("Expected null or instance of "+desiredClass.name+", got an instance of "+ptrClass.name)}ptr=ptrClass.upcast(ptr);ptrClass=ptrClass.baseClass}return ptr}function constNoSmartPtrRawPointerToWireType(destructors,handle){if(handle===null){if(this.isReference){throwBindingError("null is not a valid "+this.name)}return 0}if(!handle.$$){throwBindingError('Cannot pass "'+_embind_repr(handle)+'" as a '+this.name)}if(!handle.$$.ptr){throwBindingError("Cannot pass deleted object as a pointer of type "+this.name)}var handleClass=handle.$$.ptrType.registeredClass;var ptr=upcastPointer(handle.$$.ptr,handleClass,this.registeredClass);return ptr}function genericPointerToWireType(destructors,handle){var ptr;if(handle===null){if(this.isReference){throwBindingError("null is not a valid "+this.name)}if(this.isSmartPointer){ptr=this.rawConstructor();if(destructors!==null){destructors.push(this.rawDestructor,ptr)}return ptr}else{return 0}}if(!handle.$$){throwBindingError('Cannot pass "'+_embind_repr(handle)+'" as a '+this.name)}if(!handle.$$.ptr){throwBindingError("Cannot pass deleted object as a pointer of type "+this.name)}if(!this.isConst&&handle.$$.ptrType.isConst){throwBindingError("Cannot convert argument of type "+(handle.$$.smartPtrType?handle.$$.smartPtrType.name:handle.$$.ptrType.name)+" to parameter type "+this.name)}var handleClass=handle.$$.ptrType.registeredClass;ptr=upcastPointer(handle.$$.ptr,handleClass,this.registeredClass);if(this.isSmartPointer){if(undefined===handle.$$.smartPtr){throwBindingError("Passing raw pointer to smart pointer is illegal")}switch(this.sharingPolicy){case 0:if(handle.$$.smartPtrType===this){ptr=handle.$$.smartPtr}else{throwBindingError("Cannot convert argument of type "+(handle.$$.smartPtrType?handle.$$.smartPtrType.name:handle.$$.ptrType.name)+" to parameter type "+this.name)}break;case 1:ptr=handle.$$.smartPtr;break;case 2:if(handle.$$.smartPtrType===this){ptr=handle.$$.smartPtr}else{var clonedHandle=handle["clone"]();ptr=this.rawShare(ptr,__emval_register(function(){clonedHandle["delete"]()}));if(destructors!==null){destructors.push(this.rawDestructor,ptr)}}break;default:throwBindingError("Unsupporting sharing policy")}}return ptr}function nonConstNoSmartPtrRawPointerToWireType(destructors,handle){if(handle===null){if(this.isReference){throwBindingError("null is not a valid "+this.name)}return 0}if(!handle.$$){throwBindingError('Cannot pass "'+_embind_repr(handle)+'" as a '+this.name)}if(!handle.$$.ptr){throwBindingError("Cannot pass deleted object as a pointer of type "+this.name)}if(handle.$$.ptrType.isConst){throwBindingError("Cannot convert argument of type "+handle.$$.ptrType.name+" to parameter type "+this.name)}var handleClass=handle.$$.ptrType.registeredClass;var ptr=upcastPointer(handle.$$.ptr,handleClass,this.registeredClass);return ptr}function RegisteredPointer_getPointee(ptr){if(this.rawGetPointee){ptr=this.rawGetPointee(ptr)}return ptr}function RegisteredPointer_destructor(ptr){if(this.rawDestructor){this.rawDestructor(ptr)}}function RegisteredPointer_deleteObject(handle){if(handle!==null){handle["delete"]()}}function downcastPointer(ptr,ptrClass,desiredClass){if(ptrClass===desiredClass){return ptr}if(undefined===desiredClass.baseClass){return null}var rv=downcastPointer(ptr,ptrClass,desiredClass.baseClass);if(rv===null){return null}return desiredClass.downcast(rv)}function getInheritedInstanceCount(){return Object.keys(registeredInstances).length}function getLiveInheritedInstances(){var rv=[];for(var k in registeredInstances){if(registeredInstances.hasOwnProperty(k)){rv.push(registeredInstances[k])}}return rv}function setDelayFunction(fn){delayFunction=fn;if(deletionQueue.length&&delayFunction){delayFunction(flushPendingDeletes)}}function init_embind(){Module["getInheritedInstanceCount"]=getInheritedInstanceCount;Module["getLiveInheritedInstances"]=getLiveInheritedInstances;Module["flushPendingDeletes"]=flushPendingDeletes;Module["setDelayFunction"]=setDelayFunction}var registeredInstances={};function getBasestPointer(class_,ptr){if(ptr===undefined){throwBindingError("ptr should not be undefined")}while(class_.baseClass){ptr=class_.upcast(ptr);class_=class_.baseClass}return ptr}function getInheritedInstance(class_,ptr){ptr=getBasestPointer(class_,ptr);return registeredInstances[ptr]}function makeClassHandle(prototype,record){if(!record.ptrType||!record.ptr){throwInternalError("makeClassHandle requires ptr and ptrType")}var hasSmartPtrType=!!record.smartPtrType;var hasSmartPtr=!!record.smartPtr;if(hasSmartPtrType!==hasSmartPtr){throwInternalError("Both smartPtrType and smartPtr must be specified")}record.count={value:1};return attachFinalizer(Object.create(prototype,{$$:{value:record}}))}function RegisteredPointer_fromWireType(ptr){var rawPointer=this.getPointee(ptr);if(!rawPointer){this.destructor(ptr);return null}var registeredInstance=getInheritedInstance(this.registeredClass,rawPointer);if(undefined!==registeredInstance){if(0===registeredInstance.$$.count.value){registeredInstance.$$.ptr=rawPointer;registeredInstance.$$.smartPtr=ptr;return registeredInstance["clone"]()}else{var rv=registeredInstance["clone"]();this.destructor(ptr);return rv}}function makeDefaultHandle(){if(this.isSmartPointer){return makeClassHandle(this.registeredClass.instancePrototype,{ptrType:this.pointeeType,ptr:rawPointer,smartPtrType:this,smartPtr:ptr})}else{return makeClassHandle(this.registeredClass.instancePrototype,{ptrType:this,ptr:ptr})}}var actualType=this.registeredClass.getActualType(rawPointer);var registeredPointerRecord=registeredPointers[actualType];if(!registeredPointerRecord){return makeDefaultHandle.call(this)}var toType;if(this.isConst){toType=registeredPointerRecord.constPointerType}else{toType=registeredPointerRecord.pointerType}var dp=downcastPointer(rawPointer,this.registeredClass,toType.registeredClass);if(dp===null){return makeDefaultHandle.call(this)}if(this.isSmartPointer){return makeClassHandle(toType.registeredClass.instancePrototype,{ptrType:toType,ptr:dp,smartPtrType:this,smartPtr:ptr})}else{return makeClassHandle(toType.registeredClass.instancePrototype,{ptrType:toType,ptr:dp})}}function init_RegisteredPointer(){RegisteredPointer.prototype.getPointee=RegisteredPointer_getPointee;RegisteredPointer.prototype.destructor=RegisteredPointer_destructor;RegisteredPointer.prototype["argPackAdvance"]=8;RegisteredPointer.prototype["readValueFromPointer"]=simpleReadValueFromPointer;RegisteredPointer.prototype["deleteObject"]=RegisteredPointer_deleteObject;RegisteredPointer.prototype["fromWireType"]=RegisteredPointer_fromWireType}function RegisteredPointer(name,registeredClass,isReference,isConst,isSmartPointer,pointeeType,sharingPolicy,rawGetPointee,rawConstructor,rawShare,rawDestructor){this.name=name;this.registeredClass=registeredClass;this.isReference=isReference;this.isConst=isConst;this.isSmartPointer=isSmartPointer;this.pointeeType=pointeeType;this.sharingPolicy=sharingPolicy;this.rawGetPointee=rawGetPointee;this.rawConstructor=rawConstructor;this.rawShare=rawShare;this.rawDestructor=rawDestructor;if(!isSmartPointer&&registeredClass.baseClass===undefined){if(isConst){this["toWireType"]=constNoSmartPtrRawPointerToWireType;this.destructorFunction=null}else{this["toWireType"]=nonConstNoSmartPtrRawPointerToWireType;this.destructorFunction=null}}else{this["toWireType"]=genericPointerToWireType}}function replacePublicSymbol(name,value,numArguments){if(!Module.hasOwnProperty(name)){throwInternalError("Replacing nonexistant public symbol")}if(undefined!==Module[name].overloadTable&&undefined!==numArguments){Module[name].overloadTable[numArguments]=value}else{Module[name]=value;Module[name].argCount=numArguments}}function dynCallLegacy(sig,ptr,args){if(args&&args.length){return Module["dynCall_"+sig].apply(null,[ptr].concat(args))}return Module["dynCall_"+sig].call(null,ptr)}function dynCall(sig,ptr,args){if(sig.indexOf("j")!=-1){return dynCallLegacy(sig,ptr,args)}return wasmTable.get(ptr).apply(null,args)}function getDynCaller(sig,ptr){assert(sig.indexOf("j")>=0,"getDynCaller should only be called with i64 sigs");var argCache=[];return function(){argCache.length=arguments.length;for(var i=0;i<arguments.length;i++){argCache[i]=arguments[i]}return dynCall(sig,ptr,argCache)}}function embind__requireFunction(signature,rawFunction){signature=readLatin1String(signature);function makeDynCaller(){if(signature.indexOf("j")!=-1){return getDynCaller(signature,rawFunction)}return wasmTable.get(rawFunction)}var fp=makeDynCaller();if(typeof fp!=="function"){throwBindingError("unknown function pointer with signature "+signature+": "+rawFunction)}return fp}var UnboundTypeError=undefined;function getTypeName(type){var ptr=___getTypeName(type);var rv=readLatin1String(ptr);_free(ptr);return rv}function throwUnboundTypeError(message,types){var unboundTypes=[];var seen={};function visit(type){if(seen[type]){return}if(registeredTypes[type]){return}if(typeDependencies[type]){typeDependencies[type].forEach(visit);return}unboundTypes.push(type);seen[type]=true}types.forEach(visit);throw new UnboundTypeError(message+": "+unboundTypes.map(getTypeName).join([", "]))}function __embind_register_class(rawType,rawPointerType,rawConstPointerType,baseClassRawType,getActualTypeSignature,getActualType,upcastSignature,upcast,downcastSignature,downcast,name,destructorSignature,rawDestructor){name=readLatin1String(name);getActualType=embind__requireFunction(getActualTypeSignature,getActualType);if(upcast){upcast=embind__requireFunction(upcastSignature,upcast)}if(downcast){downcast=embind__requireFunction(downcastSignature,downcast)}rawDestructor=embind__requireFunction(destructorSignature,rawDestructor);var legalFunctionName=makeLegalFunctionName(name);exposePublicSymbol(legalFunctionName,function(){throwUnboundTypeError("Cannot construct "+name+" due to unbound types",[baseClassRawType])});whenDependentTypesAreResolved([rawType,rawPointerType,rawConstPointerType],baseClassRawType?[baseClassRawType]:[],function(base){base=base[0];var baseClass;var basePrototype;if(baseClassRawType){baseClass=base.registeredClass;basePrototype=baseClass.instancePrototype}else{basePrototype=ClassHandle.prototype}var constructor=createNamedFunction(legalFunctionName,function(){if(Object.getPrototypeOf(this)!==instancePrototype){throw new BindingError("Use 'new' to construct "+name)}if(undefined===registeredClass.constructor_body){throw new BindingError(name+" has no accessible constructor")}var body=registeredClass.constructor_body[arguments.length];if(undefined===body){throw new BindingError("Tried to invoke ctor of "+name+" with invalid number of parameters ("+arguments.length+") - expected ("+Object.keys(registeredClass.constructor_body).toString()+") parameters instead!")}return body.apply(this,arguments)});var instancePrototype=Object.create(basePrototype,{constructor:{value:constructor}});constructor.prototype=instancePrototype;var registeredClass=new RegisteredClass(name,constructor,instancePrototype,rawDestructor,baseClass,getActualType,upcast,downcast);var referenceConverter=new RegisteredPointer(name,registeredClass,true,false,false);var pointerConverter=new RegisteredPointer(name+"*",registeredClass,false,false,false);var constPointerConverter=new RegisteredPointer(name+" const*",registeredClass,false,true,false);registeredPointers[rawType]={pointerType:pointerConverter,constPointerType:constPointerConverter};replacePublicSymbol(legalFunctionName,constructor);return[referenceConverter,pointerConverter,constPointerConverter]})}function new_(constructor,argumentList){if(!(constructor instanceof Function)){throw new TypeError("new_ called with constructor type "+typeof constructor+" which is not a function")}var dummy=createNamedFunction(constructor.name||"unknownFunctionName",function(){});dummy.prototype=constructor.prototype;var obj=new dummy;var r=constructor.apply(obj,argumentList);return r instanceof Object?r:obj}function craftInvokerFunction(humanName,argTypes,classType,cppInvokerFunc,cppTargetFunc){var argCount=argTypes.length;if(argCount<2){throwBindingError("argTypes array size mismatch! Must at least get return value and 'this' types!")}var isClassMethodFunc=argTypes[1]!==null&&classType!==null;var needsDestructorStack=false;for(var i=1;i<argTypes.length;++i){if(argTypes[i]!==null&&argTypes[i].destructorFunction===undefined){needsDestructorStack=true;break}}var returns=argTypes[0].name!=="void";var argsList="";var argsListWired="";for(var i=0;i<argCount-2;++i){argsList+=(i!==0?", ":"")+"arg"+i;argsListWired+=(i!==0?", ":"")+"arg"+i+"Wired"}var invokerFnBody="return function "+makeLegalFunctionName(humanName)+"("+argsList+") {\n"+"if (arguments.length !== "+(argCount-2)+") {\n"+"throwBindingError('function "+humanName+" called with ' + arguments.length + ' arguments, expected "+(argCount-2)+" args!');\n"+"}\n";if(needsDestructorStack){invokerFnBody+="var destructors = [];\n"}var dtorStack=needsDestructorStack?"destructors":"null";var args1=["throwBindingError","invoker","fn","runDestructors","retType","classParam"];var args2=[throwBindingError,cppInvokerFunc,cppTargetFunc,runDestructors,argTypes[0],argTypes[1]];if(isClassMethodFunc){invokerFnBody+="var thisWired = classParam.toWireType("+dtorStack+", this);\n"}for(var i=0;i<argCount-2;++i){invokerFnBody+="var arg"+i+"Wired = argType"+i+".toWireType("+dtorStack+", arg"+i+"); // "+argTypes[i+2].name+"\n";args1.push("argType"+i);args2.push(argTypes[i+2])}if(isClassMethodFunc){argsListWired="thisWired"+(argsListWired.length>0?", ":"")+argsListWired}invokerFnBody+=(returns?"var rv = ":"")+"invoker(fn"+(argsListWired.length>0?", ":"")+argsListWired+");\n";if(needsDestructorStack){invokerFnBody+="runDestructors(destructors);\n"}else{for(var i=isClassMethodFunc?1:2;i<argTypes.length;++i){var paramName=i===1?"thisWired":"arg"+(i-2)+"Wired";if(argTypes[i].destructorFunction!==null){invokerFnBody+=paramName+"_dtor("+paramName+"); // "+argTypes[i].name+"\n";args1.push(paramName+"_dtor");args2.push(argTypes[i].destructorFunction)}}}if(returns){invokerFnBody+="var ret = retType.fromWireType(rv);\n"+"return ret;\n"}else{}invokerFnBody+="}\n";args1.push(invokerFnBody);var invokerFunction=new_(Function,args1).apply(null,args2);return invokerFunction}function heap32VectorToArray(count,firstElement){var array=[];for(var i=0;i<count;i++){array.push(HEAP32[(firstElement>>2)+i])}return array}function __embind_register_class_class_function(rawClassType,methodName,argCount,rawArgTypesAddr,invokerSignature,rawInvoker,fn){var rawArgTypes=heap32VectorToArray(argCount,rawArgTypesAddr);methodName=readLatin1String(methodName);rawInvoker=embind__requireFunction(invokerSignature,rawInvoker);whenDependentTypesAreResolved([],[rawClassType],function(classType){classType=classType[0];var humanName=classType.name+"."+methodName;function unboundTypesHandler(){throwUnboundTypeError("Cannot call "+humanName+" due to unbound types",rawArgTypes)}var proto=classType.registeredClass.constructor;if(undefined===proto[methodName]){unboundTypesHandler.argCount=argCount-1;proto[methodName]=unboundTypesHandler}else{ensureOverloadTable(proto,methodName,humanName);proto[methodName].overloadTable[argCount-1]=unboundTypesHandler}whenDependentTypesAreResolved([],rawArgTypes,function(argTypes){var invokerArgsArray=[argTypes[0],null].concat(argTypes.slice(1));var func=craftInvokerFunction(humanName,invokerArgsArray,null,rawInvoker,fn);if(undefined===proto[methodName].overloadTable){func.argCount=argCount-1;proto[methodName]=func}else{proto[methodName].overloadTable[argCount-1]=func}return[]});return[]})}function __embind_register_class_constructor(rawClassType,argCount,rawArgTypesAddr,invokerSignature,invoker,rawConstructor){assert(argCount>0);var rawArgTypes=heap32VectorToArray(argCount,rawArgTypesAddr);invoker=embind__requireFunction(invokerSignature,invoker);var args=[rawConstructor];var destructors=[];whenDependentTypesAreResolved([],[rawClassType],function(classType){classType=classType[0];var humanName="constructor "+classType.name;if(undefined===classType.registeredClass.constructor_body){classType.registeredClass.constructor_body=[]}if(undefined!==classType.registeredClass.constructor_body[argCount-1]){throw new BindingError("Cannot register multiple constructors with identical number of parameters ("+(argCount-1)+") for class '"+classType.name+"'! Overload resolution is currently only performed using the parameter count, not actual type info!")}classType.registeredClass.constructor_body[argCount-1]=function unboundTypeHandler(){throwUnboundTypeError("Cannot construct "+classType.name+" due to unbound types",rawArgTypes)};whenDependentTypesAreResolved([],rawArgTypes,function(argTypes){classType.registeredClass.constructor_body[argCount-1]=function constructor_body(){if(arguments.length!==argCount-1){throwBindingError(humanName+" called with "+arguments.length+" arguments, expected "+(argCount-1))}destructors.length=0;args.length=argCount;for(var i=1;i<argCount;++i){args[i]=argTypes[i]["toWireType"](destructors,arguments[i-1])}var ptr=invoker.apply(null,args);runDestructors(destructors);return argTypes[0]["fromWireType"](ptr)};return[]});return[]})}function __embind_register_class_function(rawClassType,methodName,argCount,rawArgTypesAddr,invokerSignature,rawInvoker,context,isPureVirtual){var rawArgTypes=heap32VectorToArray(argCount,rawArgTypesAddr);methodName=readLatin1String(methodName);rawInvoker=embind__requireFunction(invokerSignature,rawInvoker);whenDependentTypesAreResolved([],[rawClassType],function(classType){classType=classType[0];var humanName=classType.name+"."+methodName;if(isPureVirtual){classType.registeredClass.pureVirtualFunctions.push(methodName)}function unboundTypesHandler(){throwUnboundTypeError("Cannot call "+humanName+" due to unbound types",rawArgTypes)}var proto=classType.registeredClass.instancePrototype;var method=proto[methodName];if(undefined===method||undefined===method.overloadTable&&method.className!==classType.name&&method.argCount===argCount-2){unboundTypesHandler.argCount=argCount-2;unboundTypesHandler.className=classType.name;proto[methodName]=unboundTypesHandler}else{ensureOverloadTable(proto,methodName,humanName);proto[methodName].overloadTable[argCount-2]=unboundTypesHandler}whenDependentTypesAreResolved([],rawArgTypes,function(argTypes){var memberFunction=craftInvokerFunction(humanName,argTypes,classType,rawInvoker,context);if(undefined===proto[methodName].overloadTable){memberFunction.argCount=argCount-2;proto[methodName]=memberFunction}else{proto[methodName].overloadTable[argCount-2]=memberFunction}return[]});return[]})}function validateThis(this_,classType,humanName){if(!(this_ instanceof Object)){throwBindingError(humanName+' with invalid "this": '+this_)}if(!(this_ instanceof classType.registeredClass.constructor)){throwBindingError(humanName+' incompatible with "this" of type '+this_.constructor.name)}if(!this_.$$.ptr){throwBindingError("cannot call emscripten binding method "+humanName+" on deleted object")}return upcastPointer(this_.$$.ptr,this_.$$.ptrType.registeredClass,classType.registeredClass)}function __embind_register_class_property(classType,fieldName,getterReturnType,getterSignature,getter,getterContext,setterArgumentType,setterSignature,setter,setterContext){fieldName=readLatin1String(fieldName);getter=embind__requireFunction(getterSignature,getter);whenDependentTypesAreResolved([],[classType],function(classType){classType=classType[0];var humanName=classType.name+"."+fieldName;var desc={get:function(){throwUnboundTypeError("Cannot access "+humanName+" due to unbound types",[getterReturnType,setterArgumentType])},enumerable:true,configurable:true};if(setter){desc.set=function(){throwUnboundTypeError("Cannot access "+humanName+" due to unbound types",[getterReturnType,setterArgumentType])}}else{desc.set=function(v){throwBindingError(humanName+" is a read-only property")}}Object.defineProperty(classType.registeredClass.instancePrototype,fieldName,desc);whenDependentTypesAreResolved([],setter?[getterReturnType,setterArgumentType]:[getterReturnType],function(types){var getterReturnType=types[0];var desc={get:function(){var ptr=validateThis(this,classType,humanName+" getter");return getterReturnType["fromWireType"](getter(getterContext,ptr))},enumerable:true};if(setter){setter=embind__requireFunction(setterSignature,setter);var setterArgumentType=types[1];desc.set=function(v){var ptr=validateThis(this,classType,humanName+" setter");var destructors=[];setter(setterContext,ptr,setterArgumentType["toWireType"](destructors,v));runDestructors(destructors)}}Object.defineProperty(classType.registeredClass.instancePrototype,fieldName,desc);return[]});return[]})}var emval_free_list=[];var emval_handle_array=[{},{value:undefined},{value:null},{value:true},{value:false}];function __emval_decref(handle){if(handle>4&&0===--emval_handle_array[handle].refcount){emval_handle_array[handle]=undefined;emval_free_list.push(handle)}}function count_emval_handles(){var count=0;for(var i=5;i<emval_handle_array.length;++i){if(emval_handle_array[i]!==undefined){++count}}return count}function get_first_emval(){for(var i=5;i<emval_handle_array.length;++i){if(emval_handle_array[i]!==undefined){return emval_handle_array[i]}}return null}function init_emval(){Module["count_emval_handles"]=count_emval_handles;Module["get_first_emval"]=get_first_emval}function __emval_register(value){switch(value){case undefined:{return 1}case null:{return 2}case true:{return 3}case false:{return 4}default:{var handle=emval_free_list.length?emval_free_list.pop():emval_handle_array.length;emval_handle_array[handle]={refcount:1,value:value};return handle}}}function __embind_register_emval(rawType,name){name=readLatin1String(name);registerType(rawType,{name:name,"fromWireType":function(handle){var rv=emval_handle_array[handle].value;__emval_decref(handle);return rv},"toWireType":function(destructors,value){return __emval_register(value)},"argPackAdvance":8,"readValueFromPointer":simpleReadValueFromPointer,destructorFunction:null})}function enumReadValueFromPointer(name,shift,signed){switch(shift){case 0:return function(pointer){var heap=signed?HEAP8:HEAPU8;return this["fromWireType"](heap[pointer])};case 1:return function(pointer){var heap=signed?HEAP16:HEAPU16;return this["fromWireType"](heap[pointer>>1])};case 2:return function(pointer){var heap=signed?HEAP32:HEAPU32;return this["fromWireType"](heap[pointer>>2])};default:throw new TypeError("Unknown integer type: "+name)}}function __embind_register_enum(rawType,name,size,isSigned){var shift=getShiftFromSize(size);name=readLatin1String(name);function ctor(){}ctor.values={};registerType(rawType,{name:name,constructor:ctor,"fromWireType":function(c){return this.constructor.values[c]},"toWireType":function(destructors,c){return c.value},"argPackAdvance":8,"readValueFromPointer":enumReadValueFromPointer(name,shift,isSigned),destructorFunction:null});exposePublicSymbol(name,ctor)}function requireRegisteredType(rawType,humanName){var impl=registeredTypes[rawType];if(undefined===impl){throwBindingError(humanName+" has unknown type "+getTypeName(rawType))}return impl}function __embind_register_enum_value(rawEnumType,name,enumValue){var enumType=requireRegisteredType(rawEnumType,"enum");name=readLatin1String(name);var Enum=enumType.constructor;var Value=Object.create(enumType.constructor.prototype,{value:{value:enumValue},constructor:{value:createNamedFunction(enumType.name+"_"+name,function(){})}});Enum.values[enumValue]=Value;Enum[name]=Value}function _embind_repr(v){if(v===null){return"null"}var t=typeof v;if(t==="object"||t==="array"||t==="function"){return v.toString()}else{return""+v}}function floatReadValueFromPointer(name,shift){switch(shift){case 2:return function(pointer){return this["fromWireType"](HEAPF32[pointer>>2])};case 3:return function(pointer){return this["fromWireType"](HEAPF64[pointer>>3])};default:throw new TypeError("Unknown float type: "+name)}}function __embind_register_float(rawType,name,size){var shift=getShiftFromSize(size);name=readLatin1String(name);registerType(rawType,{name:name,"fromWireType":function(value){return value},"toWireType":function(destructors,value){if(typeof value!=="number"&&typeof value!=="boolean"){throw new TypeError('Cannot convert "'+_embind_repr(value)+'" to '+this.name)}return value},"argPackAdvance":8,"readValueFromPointer":floatReadValueFromPointer(name,shift),destructorFunction:null})}function __embind_register_function(name,argCount,rawArgTypesAddr,signature,rawInvoker,fn){var argTypes=heap32VectorToArray(argCount,rawArgTypesAddr);name=readLatin1String(name);rawInvoker=embind__requireFunction(signature,rawInvoker);exposePublicSymbol(name,function(){throwUnboundTypeError("Cannot call "+name+" due to unbound types",argTypes)},argCount-1);whenDependentTypesAreResolved([],argTypes,function(argTypes){var invokerArgsArray=[argTypes[0],null].concat(argTypes.slice(1));replacePublicSymbol(name,craftInvokerFunction(name,invokerArgsArray,null,rawInvoker,fn),argCount-1);return[]})}function integerReadValueFromPointer(name,shift,signed){switch(shift){case 0:return signed?function readS8FromPointer(pointer){return HEAP8[pointer]}:function readU8FromPointer(pointer){return HEAPU8[pointer]};case 1:return signed?function readS16FromPointer(pointer){return HEAP16[pointer>>1]}:function readU16FromPointer(pointer){return HEAPU16[pointer>>1]};case 2:return signed?function readS32FromPointer(pointer){return HEAP32[pointer>>2]}:function readU32FromPointer(pointer){return HEAPU32[pointer>>2]};default:throw new TypeError("Unknown integer type: "+name)}}function __embind_register_integer(primitiveType,name,size,minRange,maxRange){name=readLatin1String(name);if(maxRange===-1){maxRange=4294967295}var shift=getShiftFromSize(size);var fromWireType=function(value){return value};if(minRange===0){var bitshift=32-8*size;fromWireType=function(value){return value<<bitshift>>>bitshift}}var isUnsignedType=name.indexOf("unsigned")!=-1;registerType(primitiveType,{name:name,"fromWireType":fromWireType,"toWireType":function(destructors,value){if(typeof value!=="number"&&typeof value!=="boolean"){throw new TypeError('Cannot convert "'+_embind_repr(value)+'" to '+this.name)}if(value<minRange||value>maxRange){throw new TypeError('Passing a number "'+_embind_repr(value)+'" from JS side to C/C++ side to an argument of type "'+name+'", which is outside the valid range ['+minRange+", "+maxRange+"]!")}return isUnsignedType?value>>>0:value|0},"argPackAdvance":8,"readValueFromPointer":integerReadValueFromPointer(name,shift,minRange!==0),destructorFunction:null})}function __embind_register_memory_view(rawType,dataTypeIndex,name){var typeMapping=[Int8Array,Uint8Array,Int16Array,Uint16Array,Int32Array,Uint32Array,Float32Array,Float64Array];var TA=typeMapping[dataTypeIndex];function decodeMemoryView(handle){handle=handle>>2;var heap=HEAPU32;var size=heap[handle];var data=heap[handle+1];return new TA(buffer,data,size)}name=readLatin1String(name);registerType(rawType,{name:name,"fromWireType":decodeMemoryView,"argPackAdvance":8,"readValueFromPointer":decodeMemoryView},{ignoreDuplicateRegistrations:true})}function __embind_register_std_string(rawType,name){name=readLatin1String(name);var stdStringIsUTF8=name==="std::string";registerType(rawType,{name:name,"fromWireType":function(value){var length=HEAPU32[value>>2];var str;if(stdStringIsUTF8){var decodeStartPtr=value+4;for(var i=0;i<=length;++i){var currentBytePtr=value+4+i;if(i==length||HEAPU8[currentBytePtr]==0){var maxRead=currentBytePtr-decodeStartPtr;var stringSegment=UTF8ToString(decodeStartPtr,maxRead);if(str===undefined){str=stringSegment}else{str+=String.fromCharCode(0);str+=stringSegment}decodeStartPtr=currentBytePtr+1}}}else{var a=new Array(length);for(var i=0;i<length;++i){a[i]=String.fromCharCode(HEAPU8[value+4+i])}str=a.join("")}_free(value);return str},"toWireType":function(destructors,value){if(value instanceof ArrayBuffer){value=new Uint8Array(value)}var getLength;var valueIsOfTypeString=typeof value==="string";if(!(valueIsOfTypeString||value instanceof Uint8Array||value instanceof Uint8ClampedArray||value instanceof Int8Array)){throwBindingError("Cannot pass non-string to std::string")}if(stdStringIsUTF8&&valueIsOfTypeString){getLength=function(){return lengthBytesUTF8(value)}}else{getLength=function(){return value.length}}var length=getLength();var ptr=_malloc(4+length+1);HEAPU32[ptr>>2]=length;if(stdStringIsUTF8&&valueIsOfTypeString){stringToUTF8(value,ptr+4,length+1)}else{if(valueIsOfTypeString){for(var i=0;i<length;++i){var charCode=value.charCodeAt(i);if(charCode>255){_free(ptr);throwBindingError("String has UTF-16 code units that do not fit in 8 bits")}HEAPU8[ptr+4+i]=charCode}}else{for(var i=0;i<length;++i){HEAPU8[ptr+4+i]=value[i]}}}if(destructors!==null){destructors.push(_free,ptr)}return ptr},"argPackAdvance":8,"readValueFromPointer":simpleReadValueFromPointer,destructorFunction:function(ptr){_free(ptr)}})}function __embind_register_std_wstring(rawType,charSize,name){name=readLatin1String(name);var decodeString,encodeString,getHeap,lengthBytesUTF,shift;if(charSize===2){decodeString=UTF16ToString;encodeString=stringToUTF16;lengthBytesUTF=lengthBytesUTF16;getHeap=function(){return HEAPU16};shift=1}else if(charSize===4){decodeString=UTF32ToString;encodeString=stringToUTF32;lengthBytesUTF=lengthBytesUTF32;getHeap=function(){return HEAPU32};shift=2}registerType(rawType,{name:name,"fromWireType":function(value){var length=HEAPU32[value>>2];var HEAP=getHeap();var str;var decodeStartPtr=value+4;for(var i=0;i<=length;++i){var currentBytePtr=value+4+i*charSize;if(i==length||HEAP[currentBytePtr>>shift]==0){var maxReadBytes=currentBytePtr-decodeStartPtr;var stringSegment=decodeString(decodeStartPtr,maxReadBytes);if(str===undefined){str=stringSegment}else{str+=String.fromCharCode(0);str+=stringSegment}decodeStartPtr=currentBytePtr+charSize}}_free(value);return str},"toWireType":function(destructors,value){if(!(typeof value==="string")){throwBindingError("Cannot pass non-string to C++ string type "+name)}var length=lengthBytesUTF(value);var ptr=_malloc(4+length+charSize);HEAPU32[ptr>>2]=length>>shift;encodeString(value,ptr+4,length+charSize);if(destructors!==null){destructors.push(_free,ptr)}return ptr},"argPackAdvance":8,"readValueFromPointer":simpleReadValueFromPointer,destructorFunction:function(ptr){_free(ptr)}})}function __embind_register_value_array(rawType,name,constructorSignature,rawConstructor,destructorSignature,rawDestructor){tupleRegistrations[rawType]={name:readLatin1String(name),rawConstructor:embind__requireFunction(constructorSignature,rawConstructor),rawDestructor:embind__requireFunction(destructorSignature,rawDestructor),elements:[]}}function __embind_register_value_array_element(rawTupleType,getterReturnType,getterSignature,getter,getterContext,setterArgumentType,setterSignature,setter,setterContext){tupleRegistrations[rawTupleType].elements.push({getterReturnType:getterReturnType,getter:embind__requireFunction(getterSignature,getter),getterContext:getterContext,setterArgumentType:setterArgumentType,setter:embind__requireFunction(setterSignature,setter),setterContext:setterContext})}function __embind_register_value_object(rawType,name,constructorSignature,rawConstructor,destructorSignature,rawDestructor){structRegistrations[rawType]={name:readLatin1String(name),rawConstructor:embind__requireFunction(constructorSignature,rawConstructor),rawDestructor:embind__requireFunction(destructorSignature,rawDestructor),fields:[]}}function __embind_register_value_object_field(structType,fieldName,getterReturnType,getterSignature,getter,getterContext,setterArgumentType,setterSignature,setter,setterContext){structRegistrations[structType].fields.push({fieldName:readLatin1String(fieldName),getterReturnType:getterReturnType,getter:embind__requireFunction(getterSignature,getter),getterContext:getterContext,setterArgumentType:setterArgumentType,setter:embind__requireFunction(setterSignature,setter),setterContext:setterContext})}function __embind_register_void(rawType,name){name=readLatin1String(name);registerType(rawType,{isVoid:true,name:name,"argPackAdvance":0,"fromWireType":function(){return undefined},"toWireType":function(destructors,o){return undefined}})}function requireHandle(handle){if(!handle){throwBindingError("Cannot use deleted val. handle = "+handle)}return emval_handle_array[handle].value}function __emval_as(handle,returnType,destructorsRef){handle=requireHandle(handle);returnType=requireRegisteredType(returnType,"emval::as");var destructors=[];var rd=__emval_register(destructors);HEAP32[destructorsRef>>2]=rd;return returnType["toWireType"](destructors,handle)}function __emval_get_property(handle,key){handle=requireHandle(handle);key=requireHandle(key);return __emval_register(handle[key])}function __emval_incref(handle){if(handle>4){emval_handle_array[handle].refcount+=1}}var emval_symbols={};function getStringOrSymbol(address){var symbol=emval_symbols[address];if(symbol===undefined){return readLatin1String(address)}else{return symbol}}function __emval_new_cstring(v){return __emval_register(getStringOrSymbol(v))}function __emval_run_destructors(handle){var destructors=emval_handle_array[handle].value;runDestructors(destructors);__emval_decref(handle)}function __emval_take_value(type,argv){type=requireRegisteredType(type,"_emval_take_value");var v=type["readValueFromPointer"](argv);return __emval_register(v)}function _abort(){abort()}var _emscripten_get_now;if(ENVIRONMENT_IS_NODE){_emscripten_get_now=function(){var t=process["hrtime"]();return t[0]*1e3+t[1]/1e6}}else if(typeof dateNow!=="undefined"){_emscripten_get_now=dateNow}else _emscripten_get_now=function(){return performance.now()};var _emscripten_get_now_is_monotonic=true;function setErrNo(value){HEAP32[___errno_location()>>2]=value;return value}function _clock_gettime(clk_id,tp){var now;if(clk_id===0){now=Date.now()}else if((clk_id===1||clk_id===4)&&_emscripten_get_now_is_monotonic){now=_emscripten_get_now()}else{setErrNo(28);return-1}HEAP32[tp>>2]=now/1e3|0;HEAP32[tp+4>>2]=now%1e3*1e3*1e3|0;return 0}function _emscripten_asm_const_int(code,sigPtr,argbuf){var args=readAsmConstArgs(sigPtr,argbuf);return ASM_CONSTS[code].apply(null,args)}function __webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance(ctx){return!!(ctx.dibvbi=ctx.getExtension("WEBGL_draw_instanced_base_vertex_base_instance"))}function __webgl_enable_WEBGL_multi_draw_instanced_base_vertex_base_instance(ctx){return!!(ctx.mdibvbi=ctx.getExtension("WEBGL_multi_draw_instanced_base_vertex_base_instance"))}function __webgl_enable_WEBGL_multi_draw(ctx){return!!(ctx.multiDrawWebgl=ctx.getExtension("WEBGL_multi_draw"))}var GL={counter:1,buffers:[],mappedBuffers:{},programs:[],framebuffers:[],renderbuffers:[],textures:[],uniforms:[],shaders:[],vaos:[],contexts:[],offscreenCanvases:{},timerQueriesEXT:[],queries:[],samplers:[],transformFeedbacks:[],syncs:[],byteSizeByTypeRoot:5120,byteSizeByType:[1,1,2,2,4,4,4,2,3,4,8],programInfos:{},stringCache:{},stringiCache:{},unpackAlignment:4,recordError:function recordError(errorCode){if(!GL.lastError){GL.lastError=errorCode}},getNewId:function(table){var ret=GL.counter++;for(var i=table.length;i<ret;i++){table[i]=null}return ret},MAX_TEMP_BUFFER_SIZE:2097152,numTempVertexBuffersPerSize:64,log2ceilLookup:function(i){return 32-Math.clz32(i-1)},generateTempBuffers:function(quads,context){var largestIndex=GL.log2ceilLookup(GL.MAX_TEMP_BUFFER_SIZE);context.tempVertexBufferCounters1=[];context.tempVertexBufferCounters2=[];context.tempVertexBufferCounters1.length=context.tempVertexBufferCounters2.length=largestIndex+1;context.tempVertexBuffers1=[];context.tempVertexBuffers2=[];context.tempVertexBuffers1.length=context.tempVertexBuffers2.length=largestIndex+1;context.tempIndexBuffers=[];context.tempIndexBuffers.length=largestIndex+1;for(var i=0;i<=largestIndex;++i){context.tempIndexBuffers[i]=null;context.tempVertexBufferCounters1[i]=context.tempVertexBufferCounters2[i]=0;var ringbufferLength=GL.numTempVertexBuffersPerSize;context.tempVertexBuffers1[i]=[];context.tempVertexBuffers2[i]=[];var ringbuffer1=context.tempVertexBuffers1[i];var ringbuffer2=context.tempVertexBuffers2[i];ringbuffer1.length=ringbuffer2.length=ringbufferLength;for(var j=0;j<ringbufferLength;++j){ringbuffer1[j]=ringbuffer2[j]=null}}if(quads){context.tempQuadIndexBuffer=GLctx.createBuffer();context.GLctx.bindBuffer(34963,context.tempQuadIndexBuffer);var numIndexes=GL.MAX_TEMP_BUFFER_SIZE>>1;var quadIndexes=new Uint16Array(numIndexes);var i=0,v=0;while(1){quadIndexes[i++]=v;if(i>=numIndexes)break;quadIndexes[i++]=v+1;if(i>=numIndexes)break;quadIndexes[i++]=v+2;if(i>=numIndexes)break;quadIndexes[i++]=v;if(i>=numIndexes)break;quadIndexes[i++]=v+2;if(i>=numIndexes)break;quadIndexes[i++]=v+3;if(i>=numIndexes)break;v+=4}context.GLctx.bufferData(34963,quadIndexes,35044);context.GLctx.bindBuffer(34963,null)}},getTempVertexBuffer:function getTempVertexBuffer(sizeBytes){var idx=GL.log2ceilLookup(sizeBytes);var ringbuffer=GL.currentContext.tempVertexBuffers1[idx];var nextFreeBufferIndex=GL.currentContext.tempVertexBufferCounters1[idx];GL.currentContext.tempVertexBufferCounters1[idx]=GL.currentContext.tempVertexBufferCounters1[idx]+1&GL.numTempVertexBuffersPerSize-1;var vbo=ringbuffer[nextFreeBufferIndex];if(vbo){return vbo}var prevVBO=GLctx.getParameter(34964);ringbuffer[nextFreeBufferIndex]=GLctx.createBuffer();GLctx.bindBuffer(34962,ringbuffer[nextFreeBufferIndex]);GLctx.bufferData(34962,1<<idx,35048);GLctx.bindBuffer(34962,prevVBO);return ringbuffer[nextFreeBufferIndex]},getTempIndexBuffer:function getTempIndexBuffer(sizeBytes){var idx=GL.log2ceilLookup(sizeBytes);var ibo=GL.currentContext.tempIndexBuffers[idx];if(ibo){return ibo}var prevIBO=GLctx.getParameter(34965);GL.currentContext.tempIndexBuffers[idx]=GLctx.createBuffer();GLctx.bindBuffer(34963,GL.currentContext.tempIndexBuffers[idx]);GLctx.bufferData(34963,1<<idx,35048);GLctx.bindBuffer(34963,prevIBO);return GL.currentContext.tempIndexBuffers[idx]},newRenderingFrameStarted:function newRenderingFrameStarted(){if(!GL.currentContext){return}var vb=GL.currentContext.tempVertexBuffers1;GL.currentContext.tempVertexBuffers1=GL.currentContext.tempVertexBuffers2;GL.currentContext.tempVertexBuffers2=vb;vb=GL.currentContext.tempVertexBufferCounters1;GL.currentContext.tempVertexBufferCounters1=GL.currentContext.tempVertexBufferCounters2;GL.currentContext.tempVertexBufferCounters2=vb;var largestIndex=GL.log2ceilLookup(GL.MAX_TEMP_BUFFER_SIZE);for(var i=0;i<=largestIndex;++i){GL.currentContext.tempVertexBufferCounters1[i]=0}},getSource:function(shader,count,string,length){var source="";for(var i=0;i<count;++i){var len=length?HEAP32[length+i*4>>2]:-1;source+=UTF8ToString(HEAP32[string+i*4>>2],len<0?undefined:len)}return source},calcBufLength:function calcBufLength(size,type,stride,count){if(stride>0){return count*stride}var typeSize=GL.byteSizeByType[type-GL.byteSizeByTypeRoot];return size*typeSize*count},usedTempBuffers:[],preDrawHandleClientVertexAttribBindings:function preDrawHandleClientVertexAttribBindings(count){GL.resetBufferBinding=false;for(var i=0;i<GL.currentContext.maxVertexAttribs;++i){var cb=GL.currentContext.clientBuffers[i];if(!cb.clientside||!cb.enabled)continue;GL.resetBufferBinding=true;var size=GL.calcBufLength(cb.size,cb.type,cb.stride,count);var buf=GL.getTempVertexBuffer(size);GLctx.bindBuffer(34962,buf);GLctx.bufferSubData(34962,0,HEAPU8.subarray(cb.ptr,cb.ptr+size));cb.vertexAttribPointerAdaptor.call(GLctx,i,cb.size,cb.type,cb.normalized,cb.stride,0)}},postDrawHandleClientVertexAttribBindings:function postDrawHandleClientVertexAttribBindings(){if(GL.resetBufferBinding){GLctx.bindBuffer(34962,GL.buffers[GLctx.currentArrayBufferBinding])}},createContext:function(canvas,webGLContextAttributes){var ctx=canvas.getContext("webgl2",webGLContextAttributes);if(!ctx)return 0;var handle=GL.registerContext(ctx,webGLContextAttributes);return handle},registerContext:function(ctx,webGLContextAttributes){var handle=GL.getNewId(GL.contexts);var context={handle:handle,attributes:webGLContextAttributes,version:webGLContextAttributes.majorVersion,GLctx:ctx};if(ctx.canvas)ctx.canvas.GLctxObject=context;GL.contexts[handle]=context;if(typeof webGLContextAttributes.enableExtensionsByDefault==="undefined"||webGLContextAttributes.enableExtensionsByDefault){GL.initExtensions(context)}context.maxVertexAttribs=context.GLctx.getParameter(34921);context.clientBuffers=[];for(var i=0;i<context.maxVertexAttribs;i++){context.clientBuffers[i]={enabled:false,clientside:false,size:0,type:0,normalized:0,stride:0,ptr:0,vertexAttribPointerAdaptor:null}}GL.generateTempBuffers(false,context);return handle},makeContextCurrent:function(contextHandle){GL.currentContext=GL.contexts[contextHandle];Module.ctx=GLctx=GL.currentContext&&GL.currentContext.GLctx;return!(contextHandle&&!GLctx)},getContext:function(contextHandle){return GL.contexts[contextHandle]},deleteContext:function(contextHandle){if(GL.currentContext===GL.contexts[contextHandle])GL.currentContext=null;if(typeof JSEvents==="object")JSEvents.removeAllHandlersOnTarget(GL.contexts[contextHandle].GLctx.canvas);if(GL.contexts[contextHandle]&&GL.contexts[contextHandle].GLctx.canvas)GL.contexts[contextHandle].GLctx.canvas.GLctxObject=undefined;GL.contexts[contextHandle]=null},initExtensions:function(context){if(!context)context=GL.currentContext;if(context.initExtensionsDone)return;context.initExtensionsDone=true;var GLctx=context.GLctx;__webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance(GLctx);__webgl_enable_WEBGL_multi_draw_instanced_base_vertex_base_instance(GLctx);GLctx.disjointTimerQueryExt=GLctx.getExtension("EXT_disjoint_timer_query");__webgl_enable_WEBGL_multi_draw(GLctx);var exts=GLctx.getSupportedExtensions()||[];exts.forEach(function(ext){if(ext.indexOf("lose_context")<0&&ext.indexOf("debug")<0){GLctx.getExtension(ext)}})},populateUniformTable:function(program){var p=GL.programs[program];var ptable=GL.programInfos[program]={uniforms:{},maxUniformLength:0,maxAttributeLength:-1,maxUniformBlockNameLength:-1};var utable=ptable.uniforms;var numUniforms=GLctx.getProgramParameter(p,35718);for(var i=0;i<numUniforms;++i){var u=GLctx.getActiveUniform(p,i);var name=u.name;ptable.maxUniformLength=Math.max(ptable.maxUniformLength,name.length+1);if(name.slice(-1)=="]"){name=name.slice(0,name.lastIndexOf("["))}var loc=GLctx.getUniformLocation(p,name);if(loc){var id=GL.getNewId(GL.uniforms);utable[name]=[u.size,id];GL.uniforms[id]=loc;for(var j=1;j<u.size;++j){var n=name+"["+j+"]";loc=GLctx.getUniformLocation(p,n);id=GL.getNewId(GL.uniforms);GL.uniforms[id]=loc}}}}};function _emscripten_glActiveTexture(x0){GLctx["activeTexture"](x0)}function _emscripten_glAttachShader(program,shader){GLctx.attachShader(GL.programs[program],GL.shaders[shader])}function _emscripten_glBeginQuery(target,id){GLctx["beginQuery"](target,GL.queries[id])}function _emscripten_glBeginQueryEXT(target,id){GLctx.disjointTimerQueryExt["beginQueryEXT"](target,GL.timerQueriesEXT[id])}function _emscripten_glBeginTransformFeedback(x0){GLctx["beginTransformFeedback"](x0)}function _emscripten_glBindAttribLocation(program,index,name){GLctx.bindAttribLocation(GL.programs[program],index,UTF8ToString(name))}function _emscripten_glBindBuffer(target,buffer){if(target==34962){GLctx.currentArrayBufferBinding=buffer}else if(target==34963){GLctx.currentElementArrayBufferBinding=buffer}if(target==35051){GLctx.currentPixelPackBufferBinding=buffer}else if(target==35052){GLctx.currentPixelUnpackBufferBinding=buffer}GLctx.bindBuffer(target,GL.buffers[buffer])}function _emscripten_glBindBufferBase(target,index,buffer){GLctx["bindBufferBase"](target,index,GL.buffers[buffer])}function _emscripten_glBindBufferRange(target,index,buffer,offset,ptrsize){GLctx["bindBufferRange"](target,index,GL.buffers[buffer],offset,ptrsize)}function _emscripten_glBindFramebuffer(target,framebuffer){GLctx.bindFramebuffer(target,GL.framebuffers[framebuffer])}function _emscripten_glBindRenderbuffer(target,renderbuffer){GLctx.bindRenderbuffer(target,GL.renderbuffers[renderbuffer])}function _emscripten_glBindSampler(unit,sampler){GLctx["bindSampler"](unit,GL.samplers[sampler])}function _emscripten_glBindTexture(target,texture){GLctx.bindTexture(target,GL.textures[texture])}function _emscripten_glBindTransformFeedback(target,id){GLctx["bindTransformFeedback"](target,GL.transformFeedbacks[id])}function _emscripten_glBindVertexArray(vao){GLctx["bindVertexArray"](GL.vaos[vao]);var ibo=GLctx.getParameter(34965);GLctx.currentElementArrayBufferBinding=ibo?ibo.name|0:0}function _emscripten_glBindVertexArrayOES(vao){GLctx["bindVertexArray"](GL.vaos[vao]);var ibo=GLctx.getParameter(34965);GLctx.currentElementArrayBufferBinding=ibo?ibo.name|0:0}function _emscripten_glBlendColor(x0,x1,x2,x3){GLctx["blendColor"](x0,x1,x2,x3)}function _emscripten_glBlendEquation(x0){GLctx["blendEquation"](x0)}function _emscripten_glBlendEquationSeparate(x0,x1){GLctx["blendEquationSeparate"](x0,x1)}function _emscripten_glBlendFunc(x0,x1){GLctx["blendFunc"](x0,x1)}function _emscripten_glBlendFuncSeparate(x0,x1,x2,x3){GLctx["blendFuncSeparate"](x0,x1,x2,x3)}function _emscripten_glBlitFramebuffer(x0,x1,x2,x3,x4,x5,x6,x7,x8,x9){GLctx["blitFramebuffer"](x0,x1,x2,x3,x4,x5,x6,x7,x8,x9)}function _emscripten_glBufferData(target,size,data,usage){if(true){if(data){GLctx.bufferData(target,HEAPU8,usage,data,size)}else{GLctx.bufferData(target,size,usage)}}else{GLctx.bufferData(target,data?HEAPU8.subarray(data,data+size):size,usage)}}function _emscripten_glBufferSubData(target,offset,size,data){if(true){GLctx.bufferSubData(target,offset,HEAPU8,data,size);return}GLctx.bufferSubData(target,offset,HEAPU8.subarray(data,data+size))}function _emscripten_glCheckFramebufferStatus(x0){return GLctx["checkFramebufferStatus"](x0)}function _emscripten_glClear(x0){GLctx["clear"](x0)}function _emscripten_glClearBufferfi(x0,x1,x2,x3){GLctx["clearBufferfi"](x0,x1,x2,x3)}function _emscripten_glClearBufferfv(buffer,drawbuffer,value){GLctx["clearBufferfv"](buffer,drawbuffer,HEAPF32,value>>2)}function _emscripten_glClearBufferiv(buffer,drawbuffer,value){GLctx["clearBufferiv"](buffer,drawbuffer,HEAP32,value>>2)}function _emscripten_glClearBufferuiv(buffer,drawbuffer,value){GLctx["clearBufferuiv"](buffer,drawbuffer,HEAPU32,value>>2)}function _emscripten_glClearColor(x0,x1,x2,x3){GLctx["clearColor"](x0,x1,x2,x3)}function _emscripten_glClearDepthf(x0){GLctx["clearDepth"](x0)}function _emscripten_glClearStencil(x0){GLctx["clearStencil"](x0)}function convertI32PairToI53(lo,hi){return(lo>>>0)+hi*4294967296}function _emscripten_glClientWaitSync(sync,flags,timeoutLo,timeoutHi){return GLctx.clientWaitSync(GL.syncs[sync],flags,convertI32PairToI53(timeoutLo,timeoutHi))}function _emscripten_glColorMask(red,green,blue,alpha){GLctx.colorMask(!!red,!!green,!!blue,!!alpha)}function _emscripten_glCompileShader(shader){GLctx.compileShader(GL.shaders[shader])}function _emscripten_glCompressedTexImage2D(target,level,internalFormat,width,height,border,imageSize,data){if(true){if(GLctx.currentPixelUnpackBufferBinding){GLctx["compressedTexImage2D"](target,level,internalFormat,width,height,border,imageSize,data)}else{GLctx["compressedTexImage2D"](target,level,internalFormat,width,height,border,HEAPU8,data,imageSize)}return}GLctx["compressedTexImage2D"](target,level,internalFormat,width,height,border,data?HEAPU8.subarray(data,data+imageSize):null)}function _emscripten_glCompressedTexImage3D(target,level,internalFormat,width,height,depth,border,imageSize,data){if(GLctx.currentPixelUnpackBufferBinding){GLctx["compressedTexImage3D"](target,level,internalFormat,width,height,depth,border,imageSize,data)}else{GLctx["compressedTexImage3D"](target,level,internalFormat,width,height,depth,border,HEAPU8,data,imageSize)}}function _emscripten_glCompressedTexSubImage2D(target,level,xoffset,yoffset,width,height,format,imageSize,data){if(true){if(GLctx.currentPixelUnpackBufferBinding){GLctx["compressedTexSubImage2D"](target,level,xoffset,yoffset,width,height,format,imageSize,data)}else{GLctx["compressedTexSubImage2D"](target,level,xoffset,yoffset,width,height,format,HEAPU8,data,imageSize)}return}GLctx["compressedTexSubImage2D"](target,level,xoffset,yoffset,width,height,format,data?HEAPU8.subarray(data,data+imageSize):null)}function _emscripten_glCompressedTexSubImage3D(target,level,xoffset,yoffset,zoffset,width,height,depth,format,imageSize,data){if(GLctx.currentPixelUnpackBufferBinding){GLctx["compressedTexSubImage3D"](target,level,xoffset,yoffset,zoffset,width,height,depth,format,imageSize,data)}else{GLctx["compressedTexSubImage3D"](target,level,xoffset,yoffset,zoffset,width,height,depth,format,HEAPU8,data,imageSize)}}function _emscripten_glCopyBufferSubData(x0,x1,x2,x3,x4){GLctx["copyBufferSubData"](x0,x1,x2,x3,x4)}function _emscripten_glCopyTexImage2D(x0,x1,x2,x3,x4,x5,x6,x7){GLctx["copyTexImage2D"](x0,x1,x2,x3,x4,x5,x6,x7)}function _emscripten_glCopyTexSubImage2D(x0,x1,x2,x3,x4,x5,x6,x7){GLctx["copyTexSubImage2D"](x0,x1,x2,x3,x4,x5,x6,x7)}function _emscripten_glCopyTexSubImage3D(x0,x1,x2,x3,x4,x5,x6,x7,x8){GLctx["copyTexSubImage3D"](x0,x1,x2,x3,x4,x5,x6,x7,x8)}function _emscripten_glCreateProgram(){var id=GL.getNewId(GL.programs);var program=GLctx.createProgram();program.name=id;GL.programs[id]=program;return id}function _emscripten_glCreateShader(shaderType){var id=GL.getNewId(GL.shaders);GL.shaders[id]=GLctx.createShader(shaderType);return id}function _emscripten_glCullFace(x0){GLctx["cullFace"](x0)}function _emscripten_glDeleteBuffers(n,buffers){for(var i=0;i<n;i++){var id=HEAP32[buffers+i*4>>2];var buffer=GL.buffers[id];if(!buffer)continue;GLctx.deleteBuffer(buffer);buffer.name=0;GL.buffers[id]=null;if(id==GLctx.currentArrayBufferBinding)GLctx.currentArrayBufferBinding=0;if(id==GLctx.currentElementArrayBufferBinding)GLctx.currentElementArrayBufferBinding=0;if(id==GLctx.currentPixelPackBufferBinding)GLctx.currentPixelPackBufferBinding=0;if(id==GLctx.currentPixelUnpackBufferBinding)GLctx.currentPixelUnpackBufferBinding=0}}function _emscripten_glDeleteFramebuffers(n,framebuffers){for(var i=0;i<n;++i){var id=HEAP32[framebuffers+i*4>>2];var framebuffer=GL.framebuffers[id];if(!framebuffer)continue;GLctx.deleteFramebuffer(framebuffer);framebuffer.name=0;GL.framebuffers[id]=null}}function _emscripten_glDeleteProgram(id){if(!id)return;var program=GL.programs[id];if(!program){GL.recordError(1281);return}GLctx.deleteProgram(program);program.name=0;GL.programs[id]=null;GL.programInfos[id]=null}function _emscripten_glDeleteQueries(n,ids){for(var i=0;i<n;i++){var id=HEAP32[ids+i*4>>2];var query=GL.queries[id];if(!query)continue;GLctx["deleteQuery"](query);GL.queries[id]=null}}function _emscripten_glDeleteQueriesEXT(n,ids){for(var i=0;i<n;i++){var id=HEAP32[ids+i*4>>2];var query=GL.timerQueriesEXT[id];if(!query)continue;GLctx.disjointTimerQueryExt["deleteQueryEXT"](query);GL.timerQueriesEXT[id]=null}}function _emscripten_glDeleteRenderbuffers(n,renderbuffers){for(var i=0;i<n;i++){var id=HEAP32[renderbuffers+i*4>>2];var renderbuffer=GL.renderbuffers[id];if(!renderbuffer)continue;GLctx.deleteRenderbuffer(renderbuffer);renderbuffer.name=0;GL.renderbuffers[id]=null}}function _emscripten_glDeleteSamplers(n,samplers){for(var i=0;i<n;i++){var id=HEAP32[samplers+i*4>>2];var sampler=GL.samplers[id];if(!sampler)continue;GLctx["deleteSampler"](sampler);sampler.name=0;GL.samplers[id]=null}}function _emscripten_glDeleteShader(id){if(!id)return;var shader=GL.shaders[id];if(!shader){GL.recordError(1281);return}GLctx.deleteShader(shader);GL.shaders[id]=null}function _emscripten_glDeleteSync(id){if(!id)return;var sync=GL.syncs[id];if(!sync){GL.recordError(1281);return}GLctx.deleteSync(sync);sync.name=0;GL.syncs[id]=null}function _emscripten_glDeleteTextures(n,textures){for(var i=0;i<n;i++){var id=HEAP32[textures+i*4>>2];var texture=GL.textures[id];if(!texture)continue;GLctx.deleteTexture(texture);texture.name=0;GL.textures[id]=null}}function _emscripten_glDeleteTransformFeedbacks(n,ids){for(var i=0;i<n;i++){var id=HEAP32[ids+i*4>>2];var transformFeedback=GL.transformFeedbacks[id];if(!transformFeedback)continue;GLctx["deleteTransformFeedback"](transformFeedback);transformFeedback.name=0;GL.transformFeedbacks[id]=null}}function _emscripten_glDeleteVertexArrays(n,vaos){for(var i=0;i<n;i++){var id=HEAP32[vaos+i*4>>2];GLctx["deleteVertexArray"](GL.vaos[id]);GL.vaos[id]=null}}function _emscripten_glDeleteVertexArraysOES(n,vaos){for(var i=0;i<n;i++){var id=HEAP32[vaos+i*4>>2];GLctx["deleteVertexArray"](GL.vaos[id]);GL.vaos[id]=null}}function _emscripten_glDepthFunc(x0){GLctx["depthFunc"](x0)}function _emscripten_glDepthMask(flag){GLctx.depthMask(!!flag)}function _emscripten_glDepthRangef(x0,x1){GLctx["depthRange"](x0,x1)}function _emscripten_glDetachShader(program,shader){GLctx.detachShader(GL.programs[program],GL.shaders[shader])}function _emscripten_glDisable(x0){GLctx["disable"](x0)}function _emscripten_glDisableVertexAttribArray(index){var cb=GL.currentContext.clientBuffers[index];cb.enabled=false;GLctx.disableVertexAttribArray(index)}function _emscripten_glDrawArrays(mode,first,count){GL.preDrawHandleClientVertexAttribBindings(first+count);GLctx.drawArrays(mode,first,count);GL.postDrawHandleClientVertexAttribBindings()}function _emscripten_glDrawArraysInstanced(mode,first,count,primcount){GLctx["drawArraysInstanced"](mode,first,count,primcount)}function _emscripten_glDrawArraysInstancedANGLE(mode,first,count,primcount){GLctx["drawArraysInstanced"](mode,first,count,primcount)}function _emscripten_glDrawArraysInstancedARB(mode,first,count,primcount){GLctx["drawArraysInstanced"](mode,first,count,primcount)}function _emscripten_glDrawArraysInstancedEXT(mode,first,count,primcount){GLctx["drawArraysInstanced"](mode,first,count,primcount)}function _emscripten_glDrawArraysInstancedNV(mode,first,count,primcount){GLctx["drawArraysInstanced"](mode,first,count,primcount)}var tempFixedLengthArray=[];function _emscripten_glDrawBuffers(n,bufs){var bufArray=tempFixedLengthArray[n];for(var i=0;i<n;i++){bufArray[i]=HEAP32[bufs+i*4>>2]}GLctx["drawBuffers"](bufArray)}function _emscripten_glDrawBuffersEXT(n,bufs){var bufArray=tempFixedLengthArray[n];for(var i=0;i<n;i++){bufArray[i]=HEAP32[bufs+i*4>>2]}GLctx["drawBuffers"](bufArray)}function _emscripten_glDrawBuffersWEBGL(n,bufs){var bufArray=tempFixedLengthArray[n];for(var i=0;i<n;i++){bufArray[i]=HEAP32[bufs+i*4>>2]}GLctx["drawBuffers"](bufArray)}function _emscripten_glDrawElements(mode,count,type,indices){var buf;if(!GLctx.currentElementArrayBufferBinding){var size=GL.calcBufLength(1,type,0,count);buf=GL.getTempIndexBuffer(size);GLctx.bindBuffer(34963,buf);GLctx.bufferSubData(34963,0,HEAPU8.subarray(indices,indices+size));indices=0}GL.preDrawHandleClientVertexAttribBindings(count);GLctx.drawElements(mode,count,type,indices);GL.postDrawHandleClientVertexAttribBindings(count);if(!GLctx.currentElementArrayBufferBinding){GLctx.bindBuffer(34963,null)}}function _emscripten_glDrawElementsInstanced(mode,count,type,indices,primcount){GLctx["drawElementsInstanced"](mode,count,type,indices,primcount)}function _emscripten_glDrawElementsInstancedANGLE(mode,count,type,indices,primcount){GLctx["drawElementsInstanced"](mode,count,type,indices,primcount)}function _emscripten_glDrawElementsInstancedARB(mode,count,type,indices,primcount){GLctx["drawElementsInstanced"](mode,count,type,indices,primcount)}function _emscripten_glDrawElementsInstancedEXT(mode,count,type,indices,primcount){GLctx["drawElementsInstanced"](mode,count,type,indices,primcount)}function _emscripten_glDrawElementsInstancedNV(mode,count,type,indices,primcount){GLctx["drawElementsInstanced"](mode,count,type,indices,primcount)}function _glDrawElements(mode,count,type,indices){var buf;if(!GLctx.currentElementArrayBufferBinding){var size=GL.calcBufLength(1,type,0,count);buf=GL.getTempIndexBuffer(size);GLctx.bindBuffer(34963,buf);GLctx.bufferSubData(34963,0,HEAPU8.subarray(indices,indices+size));indices=0}GL.preDrawHandleClientVertexAttribBindings(count);GLctx.drawElements(mode,count,type,indices);GL.postDrawHandleClientVertexAttribBindings(count);if(!GLctx.currentElementArrayBufferBinding){GLctx.bindBuffer(34963,null)}}function _emscripten_glDrawRangeElements(mode,start,end,count,type,indices){_glDrawElements(mode,count,type,indices)}function _emscripten_glEnable(x0){GLctx["enable"](x0)}function _emscripten_glEnableVertexAttribArray(index){var cb=GL.currentContext.clientBuffers[index];cb.enabled=true;GLctx.enableVertexAttribArray(index)}function _emscripten_glEndQuery(x0){GLctx["endQuery"](x0)}function _emscripten_glEndQueryEXT(target){GLctx.disjointTimerQueryExt["endQueryEXT"](target)}function _emscripten_glEndTransformFeedback(){GLctx["endTransformFeedback"]()}function _emscripten_glFenceSync(condition,flags){var sync=GLctx.fenceSync(condition,flags);if(sync){var id=GL.getNewId(GL.syncs);sync.name=id;GL.syncs[id]=sync;return id}else{return 0}}function _emscripten_glFinish(){GLctx["finish"]()}function _emscripten_glFlush(){GLctx["flush"]()}function emscriptenWebGLGetBufferBinding(target){switch(target){case 34962:target=34964;break;case 34963:target=34965;break;case 35051:target=35053;break;case 35052:target=35055;break;case 35982:target=35983;break;case 36662:target=36662;break;case 36663:target=36663;break;case 35345:target=35368;break}var buffer=GLctx.getParameter(target);if(buffer)return buffer.name|0;else return 0}function emscriptenWebGLValidateMapBufferTarget(target){switch(target){case 34962:case 34963:case 36662:case 36663:case 35051:case 35052:case 35882:case 35982:case 35345:return true;default:return false}}function _emscripten_glFlushMappedBufferRange(target,offset,length){if(!emscriptenWebGLValidateMapBufferTarget(target)){GL.recordError(1280);err("GL_INVALID_ENUM in glFlushMappedBufferRange");return}var mapping=GL.mappedBuffers[emscriptenWebGLGetBufferBinding(target)];if(!mapping){GL.recordError(1282);err("buffer was never mapped in glFlushMappedBufferRange");return}if(!(mapping.access&16)){GL.recordError(1282);err("buffer was not mapped with GL_MAP_FLUSH_EXPLICIT_BIT in glFlushMappedBufferRange");return}if(offset<0||length<0||offset+length>mapping.length){GL.recordError(1281);err("invalid range in glFlushMappedBufferRange");return}GLctx.bufferSubData(target,mapping.offset,HEAPU8.subarray(mapping.mem+offset,mapping.mem+offset+length))}function _emscripten_glFramebufferRenderbuffer(target,attachment,renderbuffertarget,renderbuffer){GLctx.framebufferRenderbuffer(target,attachment,renderbuffertarget,GL.renderbuffers[renderbuffer])}function _emscripten_glFramebufferTexture2D(target,attachment,textarget,texture,level){GLctx.framebufferTexture2D(target,attachment,textarget,GL.textures[texture],level)}function _emscripten_glFramebufferTextureLayer(target,attachment,texture,level,layer){GLctx.framebufferTextureLayer(target,attachment,GL.textures[texture],level,layer)}function _emscripten_glFrontFace(x0){GLctx["frontFace"](x0)}function __glGenObject(n,buffers,createFunction,objectTable){for(var i=0;i<n;i++){var buffer=GLctx[createFunction]();var id=buffer&&GL.getNewId(objectTable);if(buffer){buffer.name=id;objectTable[id]=buffer}else{GL.recordError(1282)}HEAP32[buffers+i*4>>2]=id}}function _emscripten_glGenBuffers(n,buffers){__glGenObject(n,buffers,"createBuffer",GL.buffers)}function _emscripten_glGenFramebuffers(n,ids){__glGenObject(n,ids,"createFramebuffer",GL.framebuffers)}function _emscripten_glGenQueries(n,ids){__glGenObject(n,ids,"createQuery",GL.queries)}function _emscripten_glGenQueriesEXT(n,ids){for(var i=0;i<n;i++){var query=GLctx.disjointTimerQueryExt["createQueryEXT"]();if(!query){GL.recordError(1282);while(i<n)HEAP32[ids+i++*4>>2]=0;return}var id=GL.getNewId(GL.timerQueriesEXT);query.name=id;GL.timerQueriesEXT[id]=query;HEAP32[ids+i*4>>2]=id}}function _emscripten_glGenRenderbuffers(n,renderbuffers){__glGenObject(n,renderbuffers,"createRenderbuffer",GL.renderbuffers)}function _emscripten_glGenSamplers(n,samplers){__glGenObject(n,samplers,"createSampler",GL.samplers)}function _emscripten_glGenTextures(n,textures){__glGenObject(n,textures,"createTexture",GL.textures)}function _emscripten_glGenTransformFeedbacks(n,ids){__glGenObject(n,ids,"createTransformFeedback",GL.transformFeedbacks)}function _emscripten_glGenVertexArrays(n,arrays){__glGenObject(n,arrays,"createVertexArray",GL.vaos)}function _emscripten_glGenVertexArraysOES(n,arrays){__glGenObject(n,arrays,"createVertexArray",GL.vaos)}function _emscripten_glGenerateMipmap(x0){GLctx["generateMipmap"](x0)}function __glGetActiveAttribOrUniform(funcName,program,index,bufSize,length,size,type,name){program=GL.programs[program];var info=GLctx[funcName](program,index);if(info){var numBytesWrittenExclNull=name&&stringToUTF8(info.name,name,bufSize);if(length)HEAP32[length>>2]=numBytesWrittenExclNull;if(size)HEAP32[size>>2]=info.size;if(type)HEAP32[type>>2]=info.type}}function _emscripten_glGetActiveAttrib(program,index,bufSize,length,size,type,name){__glGetActiveAttribOrUniform("getActiveAttrib",program,index,bufSize,length,size,type,name)}function _emscripten_glGetActiveUniform(program,index,bufSize,length,size,type,name){__glGetActiveAttribOrUniform("getActiveUniform",program,index,bufSize,length,size,type,name)}function _emscripten_glGetActiveUniformBlockName(program,uniformBlockIndex,bufSize,length,uniformBlockName){program=GL.programs[program];var result=GLctx["getActiveUniformBlockName"](program,uniformBlockIndex);if(!result)return;if(uniformBlockName&&bufSize>0){var numBytesWrittenExclNull=stringToUTF8(result,uniformBlockName,bufSize);if(length)HEAP32[length>>2]=numBytesWrittenExclNull}else{if(length)HEAP32[length>>2]=0}}function _emscripten_glGetActiveUniformBlockiv(program,uniformBlockIndex,pname,params){if(!params){GL.recordError(1281);return}program=GL.programs[program];switch(pname){case 35393:var name=GLctx["getActiveUniformBlockName"](program,uniformBlockIndex);HEAP32[params>>2]=name.length+1;return;default:var result=GLctx["getActiveUniformBlockParameter"](program,uniformBlockIndex,pname);if(!result)return;if(typeof result=="number"){HEAP32[params>>2]=result}else{for(var i=0;i<result.length;i++){HEAP32[params+i*4>>2]=result[i]}}}}function _emscripten_glGetActiveUniformsiv(program,uniformCount,uniformIndices,pname,params){if(!params){GL.recordError(1281);return}if(uniformCount>0&&uniformIndices==0){GL.recordError(1281);return}program=GL.programs[program];var ids=[];for(var i=0;i<uniformCount;i++){ids.push(HEAP32[uniformIndices+i*4>>2])}var result=GLctx["getActiveUniforms"](program,ids,pname);if(!result)return;var len=result.length;for(var i=0;i<len;i++){HEAP32[params+i*4>>2]=result[i]}}function _emscripten_glGetAttachedShaders(program,maxCount,count,shaders){var result=GLctx.getAttachedShaders(GL.programs[program]);var len=result.length;if(len>maxCount){len=maxCount}HEAP32[count>>2]=len;for(var i=0;i<len;++i){var id=GL.shaders.indexOf(result[i]);HEAP32[shaders+i*4>>2]=id}}function _emscripten_glGetAttribLocation(program,name){return GLctx.getAttribLocation(GL.programs[program],UTF8ToString(name))}function writeI53ToI64(ptr,num){HEAPU32[ptr>>2]=num;HEAPU32[ptr+4>>2]=(num-HEAPU32[ptr>>2])/4294967296}function emscriptenWebGLGet(name_,p,type){if(!p){GL.recordError(1281);return}var ret=undefined;switch(name_){case 36346:ret=1;break;case 36344:if(type!=0&&type!=1){GL.recordError(1280)}return;case 34814:case 36345:ret=0;break;case 34466:var formats=GLctx.getParameter(34467);ret=formats?formats.length:0;break;case 33309:if(GL.currentContext.version<2){GL.recordError(1282);return}var exts=GLctx.getSupportedExtensions()||[];ret=2*exts.length;break;case 33307:case 33308:if(GL.currentContext.version<2){GL.recordError(1280);return}ret=name_==33307?3:0;break}if(ret===undefined){var result=GLctx.getParameter(name_);switch(typeof result){case"number":ret=result;break;case"boolean":ret=result?1:0;break;case"string":GL.recordError(1280);return;case"object":if(result===null){switch(name_){case 34964:case 35725:case 34965:case 36006:case 36007:case 32873:case 34229:case 36662:case 36663:case 35053:case 35055:case 36010:case 35097:case 35869:case 32874:case 36389:case 35983:case 35368:case 34068:{ret=0;break}default:{GL.recordError(1280);return}}}else if(result instanceof Float32Array||result instanceof Uint32Array||result instanceof Int32Array||result instanceof Array){for(var i=0;i<result.length;++i){switch(type){case 0:HEAP32[p+i*4>>2]=result[i];break;case 2:HEAPF32[p+i*4>>2]=result[i];break;case 4:HEAP8[p+i>>0]=result[i]?1:0;break}}return}else{try{ret=result.name|0}catch(e){GL.recordError(1280);err("GL_INVALID_ENUM in glGet"+type+"v: Unknown object returned from WebGL getParameter("+name_+")! (error: "+e+")");return}}break;default:GL.recordError(1280);err("GL_INVALID_ENUM in glGet"+type+"v: Native code calling glGet"+type+"v("+name_+") and it returns "+result+" of type "+typeof result+"!");return}}switch(type){case 1:writeI53ToI64(p,ret);break;case 0:HEAP32[p>>2]=ret;break;case 2:HEAPF32[p>>2]=ret;break;case 4:HEAP8[p>>0]=ret?1:0;break}}function _emscripten_glGetBooleanv(name_,p){emscriptenWebGLGet(name_,p,4)}function _emscripten_glGetBufferParameteri64v(target,value,data){if(!data){GL.recordError(1281);return}writeI53ToI64(data,GLctx.getBufferParameter(target,value))}function _emscripten_glGetBufferParameteriv(target,value,data){if(!data){GL.recordError(1281);return}HEAP32[data>>2]=GLctx.getBufferParameter(target,value)}function _emscripten_glGetBufferPointerv(target,pname,params){if(pname==35005){var ptr=0;var mappedBuffer=GL.mappedBuffers[emscriptenWebGLGetBufferBinding(target)];if(mappedBuffer){ptr=mappedBuffer.mem}HEAP32[params>>2]=ptr}else{GL.recordError(1280);err("GL_INVALID_ENUM in glGetBufferPointerv")}}function _emscripten_glGetError(){var error=GLctx.getError()||GL.lastError;GL.lastError=0;return error}function _emscripten_glGetFloatv(name_,p){emscriptenWebGLGet(name_,p,2)}function _emscripten_glGetFragDataLocation(program,name){return GLctx["getFragDataLocation"](GL.programs[program],UTF8ToString(name))}function _emscripten_glGetFramebufferAttachmentParameteriv(target,attachment,pname,params){var result=GLctx.getFramebufferAttachmentParameter(target,attachment,pname);if(result instanceof WebGLRenderbuffer||result instanceof WebGLTexture){result=result.name|0}HEAP32[params>>2]=result}function emscriptenWebGLGetIndexed(target,index,data,type){if(!data){GL.recordError(1281);return}var result=GLctx["getIndexedParameter"](target,index);var ret;switch(typeof result){case"boolean":ret=result?1:0;break;case"number":ret=result;break;case"object":if(result===null){switch(target){case 35983:case 35368:ret=0;break;default:{GL.recordError(1280);return}}}else if(result instanceof WebGLBuffer){ret=result.name|0}else{GL.recordError(1280);return}break;default:GL.recordError(1280);return}switch(type){case 1:writeI53ToI64(data,ret);break;case 0:HEAP32[data>>2]=ret;break;case 2:HEAPF32[data>>2]=ret;break;case 4:HEAP8[data>>0]=ret?1:0;break;default:throw"internal emscriptenWebGLGetIndexed() error, bad type: "+type}}function _emscripten_glGetInteger64i_v(target,index,data){emscriptenWebGLGetIndexed(target,index,data,1)}function _emscripten_glGetInteger64v(name_,p){emscriptenWebGLGet(name_,p,1)}function _emscripten_glGetIntegeri_v(target,index,data){emscriptenWebGLGetIndexed(target,index,data,0)}function _emscripten_glGetIntegerv(name_,p){emscriptenWebGLGet(name_,p,0)}function _emscripten_glGetInternalformativ(target,internalformat,pname,bufSize,params){if(bufSize<0){GL.recordError(1281);return}if(!params){GL.recordError(1281);return}var ret=GLctx["getInternalformatParameter"](target,internalformat,pname);if(ret===null)return;for(var i=0;i<ret.length&&i<bufSize;++i){HEAP32[params+i>>2]=ret[i]}}function _emscripten_glGetProgramBinary(program,bufSize,length,binaryFormat,binary){GL.recordError(1282)}function _emscripten_glGetProgramInfoLog(program,maxLength,length,infoLog){var log=GLctx.getProgramInfoLog(GL.programs[program]);if(log===null)log="(unknown error)";var numBytesWrittenExclNull=maxLength>0&&infoLog?stringToUTF8(log,infoLog,maxLength):0;if(length)HEAP32[length>>2]=numBytesWrittenExclNull}function _emscripten_glGetProgramiv(program,pname,p){if(!p){GL.recordError(1281);return}if(program>=GL.counter){GL.recordError(1281);return}var ptable=GL.programInfos[program];if(!ptable){GL.recordError(1282);return}if(pname==35716){var log=GLctx.getProgramInfoLog(GL.programs[program]);if(log===null)log="(unknown error)";HEAP32[p>>2]=log.length+1}else if(pname==35719){HEAP32[p>>2]=ptable.maxUniformLength}else if(pname==35722){if(ptable.maxAttributeLength==-1){program=GL.programs[program];var numAttribs=GLctx.getProgramParameter(program,35721);ptable.maxAttributeLength=0;for(var i=0;i<numAttribs;++i){var activeAttrib=GLctx.getActiveAttrib(program,i);ptable.maxAttributeLength=Math.max(ptable.maxAttributeLength,activeAttrib.name.length+1)}}HEAP32[p>>2]=ptable.maxAttributeLength}else if(pname==35381){if(ptable.maxUniformBlockNameLength==-1){program=GL.programs[program];var numBlocks=GLctx.getProgramParameter(program,35382);ptable.maxUniformBlockNameLength=0;for(var i=0;i<numBlocks;++i){var activeBlockName=GLctx.getActiveUniformBlockName(program,i);ptable.maxUniformBlockNameLength=Math.max(ptable.maxUniformBlockNameLength,activeBlockName.length+1)}}HEAP32[p>>2]=ptable.maxUniformBlockNameLength}else{HEAP32[p>>2]=GLctx.getProgramParameter(GL.programs[program],pname)}}function _emscripten_glGetQueryObjecti64vEXT(id,pname,params){if(!params){GL.recordError(1281);return}var query=GL.timerQueriesEXT[id];var param=GLctx.disjointTimerQueryExt["getQueryObjectEXT"](query,pname);var ret;if(typeof param=="boolean"){ret=param?1:0}else{ret=param}writeI53ToI64(params,ret)}function _emscripten_glGetQueryObjectivEXT(id,pname,params){if(!params){GL.recordError(1281);return}var query=GL.timerQueriesEXT[id];var param=GLctx.disjointTimerQueryExt["getQueryObjectEXT"](query,pname);var ret;if(typeof param=="boolean"){ret=param?1:0}else{ret=param}HEAP32[params>>2]=ret}function _emscripten_glGetQueryObjectui64vEXT(id,pname,params){if(!params){GL.recordError(1281);return}var query=GL.timerQueriesEXT[id];var param=GLctx.disjointTimerQueryExt["getQueryObjectEXT"](query,pname);var ret;if(typeof param=="boolean"){ret=param?1:0}else{ret=param}writeI53ToI64(params,ret)}function _emscripten_glGetQueryObjectuiv(id,pname,params){if(!params){GL.recordError(1281);return}var query=GL.queries[id];var param=GLctx["getQueryParameter"](query,pname);var ret;if(typeof param=="boolean"){ret=param?1:0}else{ret=param}HEAP32[params>>2]=ret}function _emscripten_glGetQueryObjectuivEXT(id,pname,params){if(!params){GL.recordError(1281);return}var query=GL.timerQueriesEXT[id];var param=GLctx.disjointTimerQueryExt["getQueryObjectEXT"](query,pname);var ret;if(typeof param=="boolean"){ret=param?1:0}else{ret=param}HEAP32[params>>2]=ret}function _emscripten_glGetQueryiv(target,pname,params){if(!params){GL.recordError(1281);return}HEAP32[params>>2]=GLctx["getQuery"](target,pname)}function _emscripten_glGetQueryivEXT(target,pname,params){if(!params){GL.recordError(1281);return}HEAP32[params>>2]=GLctx.disjointTimerQueryExt["getQueryEXT"](target,pname)}function _emscripten_glGetRenderbufferParameteriv(target,pname,params){if(!params){GL.recordError(1281);return}HEAP32[params>>2]=GLctx.getRenderbufferParameter(target,pname)}function _emscripten_glGetSamplerParameterfv(sampler,pname,params){if(!params){GL.recordError(1281);return}sampler=GL.samplers[sampler];HEAPF32[params>>2]=GLctx["getSamplerParameter"](sampler,pname)}function _emscripten_glGetSamplerParameteriv(sampler,pname,params){if(!params){GL.recordError(1281);return}sampler=GL.samplers[sampler];HEAP32[params>>2]=GLctx["getSamplerParameter"](sampler,pname)}function _emscripten_glGetShaderInfoLog(shader,maxLength,length,infoLog){var log=GLctx.getShaderInfoLog(GL.shaders[shader]);if(log===null)log="(unknown error)";var numBytesWrittenExclNull=maxLength>0&&infoLog?stringToUTF8(log,infoLog,maxLength):0;if(length)HEAP32[length>>2]=numBytesWrittenExclNull}function _emscripten_glGetShaderPrecisionFormat(shaderType,precisionType,range,precision){var result=GLctx.getShaderPrecisionFormat(shaderType,precisionType);HEAP32[range>>2]=result.rangeMin;HEAP32[range+4>>2]=result.rangeMax;HEAP32[precision>>2]=result.precision}function _emscripten_glGetShaderSource(shader,bufSize,length,source){var result=GLctx.getShaderSource(GL.shaders[shader]);if(!result)return;var numBytesWrittenExclNull=bufSize>0&&source?stringToUTF8(result,source,bufSize):0;if(length)HEAP32[length>>2]=numBytesWrittenExclNull}function _emscripten_glGetShaderiv(shader,pname,p){if(!p){GL.recordError(1281);return}if(pname==35716){var log=GLctx.getShaderInfoLog(GL.shaders[shader]);if(log===null)log="(unknown error)";var logLength=log?log.length+1:0;HEAP32[p>>2]=logLength}else if(pname==35720){var source=GLctx.getShaderSource(GL.shaders[shader]);var sourceLength=source?source.length+1:0;HEAP32[p>>2]=sourceLength}else{HEAP32[p>>2]=GLctx.getShaderParameter(GL.shaders[shader],pname)}}function stringToNewUTF8(jsString){var length=lengthBytesUTF8(jsString)+1;var cString=_malloc(length);stringToUTF8(jsString,cString,length);return cString}function _emscripten_glGetString(name_){if(GL.stringCache[name_])return GL.stringCache[name_];var ret;switch(name_){case 7939:var exts=GLctx.getSupportedExtensions()||[];exts=exts.concat(exts.map(function(e){return"GL_"+e}));ret=stringToNewUTF8(exts.join(" "));break;case 7936:case 7937:case 37445:case 37446:var s=GLctx.getParameter(name_);if(!s){GL.recordError(1280)}ret=stringToNewUTF8(s);break;case 7938:var glVersion=GLctx.getParameter(7938);if(true)glVersion="OpenGL ES 3.0 ("+glVersion+")";else{glVersion="OpenGL ES 2.0 ("+glVersion+")"}ret=stringToNewUTF8(glVersion);break;case 35724:var glslVersion=GLctx.getParameter(35724);var ver_re=/^WebGL GLSL ES ([0-9]\.[0-9][0-9]?)(?:$| .*)/;var ver_num=glslVersion.match(ver_re);if(ver_num!==null){if(ver_num[1].length==3)ver_num[1]=ver_num[1]+"0";glslVersion="OpenGL ES GLSL ES "+ver_num[1]+" ("+glslVersion+")"}ret=stringToNewUTF8(glslVersion);break;default:GL.recordError(1280);return 0}GL.stringCache[name_]=ret;return ret}function _emscripten_glGetStringi(name,index){if(GL.currentContext.version<2){GL.recordError(1282);return 0}var stringiCache=GL.stringiCache[name];if(stringiCache){if(index<0||index>=stringiCache.length){GL.recordError(1281);return 0}return stringiCache[index]}switch(name){case 7939:var exts=GLctx.getSupportedExtensions()||[];exts=exts.concat(exts.map(function(e){return"GL_"+e}));exts=exts.map(function(e){return stringToNewUTF8(e)});stringiCache=GL.stringiCache[name]=exts;if(index<0||index>=stringiCache.length){GL.recordError(1281);return 0}return stringiCache[index];default:GL.recordError(1280);return 0}}function _emscripten_glGetSynciv(sync,pname,bufSize,length,values){if(bufSize<0){GL.recordError(1281);return}if(!values){GL.recordError(1281);return}var ret=GLctx.getSyncParameter(GL.syncs[sync],pname);HEAP32[length>>2]=ret;if(ret!==null&&length)HEAP32[length>>2]=1}function _emscripten_glGetTexParameterfv(target,pname,params){if(!params){GL.recordError(1281);return}HEAPF32[params>>2]=GLctx.getTexParameter(target,pname)}function _emscripten_glGetTexParameteriv(target,pname,params){if(!params){GL.recordError(1281);return}HEAP32[params>>2]=GLctx.getTexParameter(target,pname)}function _emscripten_glGetTransformFeedbackVarying(program,index,bufSize,length,size,type,name){program=GL.programs[program];var info=GLctx["getTransformFeedbackVarying"](program,index);if(!info)return;if(name&&bufSize>0){var numBytesWrittenExclNull=stringToUTF8(info.name,name,bufSize);if(length)HEAP32[length>>2]=numBytesWrittenExclNull}else{if(length)HEAP32[length>>2]=0}if(size)HEAP32[size>>2]=info.size;if(type)HEAP32[type>>2]=info.type}function _emscripten_glGetUniformBlockIndex(program,uniformBlockName){return GLctx["getUniformBlockIndex"](GL.programs[program],UTF8ToString(uniformBlockName))}function _emscripten_glGetUniformIndices(program,uniformCount,uniformNames,uniformIndices){if(!uniformIndices){GL.recordError(1281);return}if(uniformCount>0&&(uniformNames==0||uniformIndices==0)){GL.recordError(1281);return}program=GL.programs[program];var names=[];for(var i=0;i<uniformCount;i++)names.push(UTF8ToString(HEAP32[uniformNames+i*4>>2]));var result=GLctx["getUniformIndices"](program,names);if(!result)return;var len=result.length;for(var i=0;i<len;i++){HEAP32[uniformIndices+i*4>>2]=result[i]}}function jstoi_q(str){return parseInt(str)}function _emscripten_glGetUniformLocation(program,name){name=UTF8ToString(name);var arrayIndex=0;if(name[name.length-1]=="]"){var leftBrace=name.lastIndexOf("[");arrayIndex=name[leftBrace+1]!="]"?jstoi_q(name.slice(leftBrace+1)):0;name=name.slice(0,leftBrace)}var uniformInfo=GL.programInfos[program]&&GL.programInfos[program].uniforms[name];if(uniformInfo&&arrayIndex>=0&&arrayIndex<uniformInfo[0]){return uniformInfo[1]+arrayIndex}else{return-1}}function emscriptenWebGLGetUniform(program,location,params,type){if(!params){GL.recordError(1281);return}var data=GLctx.getUniform(GL.programs[program],GL.uniforms[location]);if(typeof data=="number"||typeof data=="boolean"){switch(type){case 0:HEAP32[params>>2]=data;break;case 2:HEAPF32[params>>2]=data;break}}else{for(var i=0;i<data.length;i++){switch(type){case 0:HEAP32[params+i*4>>2]=data[i];break;case 2:HEAPF32[params+i*4>>2]=data[i];break}}}}function _emscripten_glGetUniformfv(program,location,params){emscriptenWebGLGetUniform(program,location,params,2)}function _emscripten_glGetUniformiv(program,location,params){emscriptenWebGLGetUniform(program,location,params,0)}function _emscripten_glGetUniformuiv(program,location,params){emscriptenWebGLGetUniform(program,location,params,0)}function emscriptenWebGLGetVertexAttrib(index,pname,params,type){if(!params){GL.recordError(1281);return}if(GL.currentContext.clientBuffers[index].enabled){err("glGetVertexAttrib*v on client-side array: not supported, bad data returned")}var data=GLctx.getVertexAttrib(index,pname);if(pname==34975){HEAP32[params>>2]=data&&data["name"]}else if(typeof data=="number"||typeof data=="boolean"){switch(type){case 0:HEAP32[params>>2]=data;break;case 2:HEAPF32[params>>2]=data;break;case 5:HEAP32[params>>2]=Math.fround(data);break}}else{for(var i=0;i<data.length;i++){switch(type){case 0:HEAP32[params+i*4>>2]=data[i];break;case 2:HEAPF32[params+i*4>>2]=data[i];break;case 5:HEAP32[params+i*4>>2]=Math.fround(data[i]);break}}}}function _emscripten_glGetVertexAttribIiv(index,pname,params){emscriptenWebGLGetVertexAttrib(index,pname,params,0)}function _emscripten_glGetVertexAttribIuiv(index,pname,params){emscriptenWebGLGetVertexAttrib(index,pname,params,0)}function _emscripten_glGetVertexAttribPointerv(index,pname,pointer){if(!pointer){GL.recordError(1281);return}if(GL.currentContext.clientBuffers[index].enabled){err("glGetVertexAttribPointer on client-side array: not supported, bad data returned")}HEAP32[pointer>>2]=GLctx.getVertexAttribOffset(index,pname)}function _emscripten_glGetVertexAttribfv(index,pname,params){emscriptenWebGLGetVertexAttrib(index,pname,params,2)}function _emscripten_glGetVertexAttribiv(index,pname,params){emscriptenWebGLGetVertexAttrib(index,pname,params,5)}function _emscripten_glHint(x0,x1){GLctx["hint"](x0,x1)}function _emscripten_glInvalidateFramebuffer(target,numAttachments,attachments){var list=tempFixedLengthArray[numAttachments];for(var i=0;i<numAttachments;i++){list[i]=HEAP32[attachments+i*4>>2]}GLctx["invalidateFramebuffer"](target,list)}function _emscripten_glInvalidateSubFramebuffer(target,numAttachments,attachments,x,y,width,height){var list=tempFixedLengthArray[numAttachments];for(var i=0;i<numAttachments;i++){list[i]=HEAP32[attachments+i*4>>2]}GLctx["invalidateSubFramebuffer"](target,list,x,y,width,height)}function _emscripten_glIsBuffer(buffer){var b=GL.buffers[buffer];if(!b)return 0;return GLctx.isBuffer(b)}function _emscripten_glIsEnabled(x0){return GLctx["isEnabled"](x0)}function _emscripten_glIsFramebuffer(framebuffer){var fb=GL.framebuffers[framebuffer];if(!fb)return 0;return GLctx.isFramebuffer(fb)}function _emscripten_glIsProgram(program){program=GL.programs[program];if(!program)return 0;return GLctx.isProgram(program)}function _emscripten_glIsQuery(id){var query=GL.queries[id];if(!query)return 0;return GLctx["isQuery"](query)}function _emscripten_glIsQueryEXT(id){var query=GL.timerQueriesEXT[id];if(!query)return 0;return GLctx.disjointTimerQueryExt["isQueryEXT"](query)}function _emscripten_glIsRenderbuffer(renderbuffer){var rb=GL.renderbuffers[renderbuffer];if(!rb)return 0;return GLctx.isRenderbuffer(rb)}function _emscripten_glIsSampler(id){var sampler=GL.samplers[id];if(!sampler)return 0;return GLctx["isSampler"](sampler)}function _emscripten_glIsShader(shader){var s=GL.shaders[shader];if(!s)return 0;return GLctx.isShader(s)}function _emscripten_glIsSync(sync){return GLctx.isSync(GL.syncs[sync])}function _emscripten_glIsTexture(id){var texture=GL.textures[id];if(!texture)return 0;return GLctx.isTexture(texture)}function _emscripten_glIsTransformFeedback(id){return GLctx["isTransformFeedback"](GL.transformFeedbacks[id])}function _emscripten_glIsVertexArray(array){var vao=GL.vaos[array];if(!vao)return 0;return GLctx["isVertexArray"](vao)}function _emscripten_glIsVertexArrayOES(array){var vao=GL.vaos[array];if(!vao)return 0;return GLctx["isVertexArray"](vao)}function _emscripten_glLineWidth(x0){GLctx["lineWidth"](x0)}function _emscripten_glLinkProgram(program){GLctx.linkProgram(GL.programs[program]);GL.populateUniformTable(program)}function _emscripten_glMapBufferRange(target,offset,length,access){if(access!=26&&access!=10){err("glMapBufferRange is only supported when access is MAP_WRITE|INVALIDATE_BUFFER");return 0}if(!emscriptenWebGLValidateMapBufferTarget(target)){GL.recordError(1280);err("GL_INVALID_ENUM in glMapBufferRange");return 0}var mem=_malloc(length);if(!mem)return 0;GL.mappedBuffers[emscriptenWebGLGetBufferBinding(target)]={offset:offset,length:length,mem:mem,access:access};return mem}function _emscripten_glPauseTransformFeedback(){GLctx["pauseTransformFeedback"]()}function _emscripten_glPixelStorei(pname,param){if(pname==3317){GL.unpackAlignment=param}GLctx.pixelStorei(pname,param)}function _emscripten_glPolygonOffset(x0,x1){GLctx["polygonOffset"](x0,x1)}function _emscripten_glProgramBinary(program,binaryFormat,binary,length){GL.recordError(1280)}function _emscripten_glProgramParameteri(program,pname,value){GL.recordError(1280)}function _emscripten_glQueryCounterEXT(id,target){GLctx.disjointTimerQueryExt["queryCounterEXT"](GL.timerQueriesEXT[id],target)}function _emscripten_glReadBuffer(x0){GLctx["readBuffer"](x0)}function computeUnpackAlignedImageSize(width,height,sizePerPixel,alignment){function roundedToNextMultipleOf(x,y){return x+y-1&-y}var plainRowSize=width*sizePerPixel;var alignedRowSize=roundedToNextMultipleOf(plainRowSize,alignment);return height*alignedRowSize}function __colorChannelsInGlTextureFormat(format){var colorChannels={5:3,6:4,8:2,29502:3,29504:4,26917:2,26918:2,29846:3,29847:4};return colorChannels[format-6402]||1}function heapObjectForWebGLType(type){type-=5120;if(type==0)return HEAP8;if(type==1)return HEAPU8;if(type==2)return HEAP16;if(type==4)return HEAP32;if(type==6)return HEAPF32;if(type==5||type==28922||type==28520||type==30779||type==30782)return HEAPU32;return HEAPU16}function heapAccessShiftForWebGLHeap(heap){return 31-Math.clz32(heap.BYTES_PER_ELEMENT)}function emscriptenWebGLGetTexPixelData(type,format,width,height,pixels,internalFormat){var heap=heapObjectForWebGLType(type);var shift=heapAccessShiftForWebGLHeap(heap);var byteSize=1<<shift;var sizePerPixel=__colorChannelsInGlTextureFormat(format)*byteSize;var bytes=computeUnpackAlignedImageSize(width,height,sizePerPixel,GL.unpackAlignment);return heap.subarray(pixels>>shift,pixels+bytes>>shift)}function _emscripten_glReadPixels(x,y,width,height,format,type,pixels){if(true){if(GLctx.currentPixelPackBufferBinding){GLctx.readPixels(x,y,width,height,format,type,pixels)}else{var heap=heapObjectForWebGLType(type);GLctx.readPixels(x,y,width,height,format,type,heap,pixels>>heapAccessShiftForWebGLHeap(heap))}return}var pixelData=emscriptenWebGLGetTexPixelData(type,format,width,height,pixels,format);if(!pixelData){GL.recordError(1280);return}GLctx.readPixels(x,y,width,height,format,type,pixelData)}function _emscripten_glReleaseShaderCompiler(){}function _emscripten_glRenderbufferStorage(x0,x1,x2,x3){GLctx["renderbufferStorage"](x0,x1,x2,x3)}function _emscripten_glRenderbufferStorageMultisample(x0,x1,x2,x3,x4){GLctx["renderbufferStorageMultisample"](x0,x1,x2,x3,x4)}function _emscripten_glResumeTransformFeedback(){GLctx["resumeTransformFeedback"]()}function _emscripten_glSampleCoverage(value,invert){GLctx.sampleCoverage(value,!!invert)}function _emscripten_glSamplerParameterf(sampler,pname,param){GLctx["samplerParameterf"](GL.samplers[sampler],pname,param)}function _emscripten_glSamplerParameterfv(sampler,pname,params){var param=HEAPF32[params>>2];GLctx["samplerParameterf"](GL.samplers[sampler],pname,param)}function _emscripten_glSamplerParameteri(sampler,pname,param){GLctx["samplerParameteri"](GL.samplers[sampler],pname,param)}function _emscripten_glSamplerParameteriv(sampler,pname,params){var param=HEAP32[params>>2];GLctx["samplerParameteri"](GL.samplers[sampler],pname,param)}function _emscripten_glScissor(x0,x1,x2,x3){GLctx["scissor"](x0,x1,x2,x3)}function _emscripten_glShaderBinary(){GL.recordError(1280)}function _emscripten_glShaderSource(shader,count,string,length){var source=GL.getSource(shader,count,string,length);GLctx.shaderSource(GL.shaders[shader],source)}function _emscripten_glStencilFunc(x0,x1,x2){GLctx["stencilFunc"](x0,x1,x2)}function _emscripten_glStencilFuncSeparate(x0,x1,x2,x3){GLctx["stencilFuncSeparate"](x0,x1,x2,x3)}function _emscripten_glStencilMask(x0){GLctx["stencilMask"](x0)}function _emscripten_glStencilMaskSeparate(x0,x1){GLctx["stencilMaskSeparate"](x0,x1)}function _emscripten_glStencilOp(x0,x1,x2){GLctx["stencilOp"](x0,x1,x2)}function _emscripten_glStencilOpSeparate(x0,x1,x2,x3){GLctx["stencilOpSeparate"](x0,x1,x2,x3)}function _emscripten_glTexImage2D(target,level,internalFormat,width,height,border,format,type,pixels){if(true){if(GLctx.currentPixelUnpackBufferBinding){GLctx.texImage2D(target,level,internalFormat,width,height,border,format,type,pixels)}else if(pixels){var heap=heapObjectForWebGLType(type);GLctx.texImage2D(target,level,internalFormat,width,height,border,format,type,heap,pixels>>heapAccessShiftForWebGLHeap(heap))}else{GLctx.texImage2D(target,level,internalFormat,width,height,border,format,type,null)}return}GLctx.texImage2D(target,level,internalFormat,width,height,border,format,type,pixels?emscriptenWebGLGetTexPixelData(type,format,width,height,pixels,internalFormat):null)}function _emscripten_glTexImage3D(target,level,internalFormat,width,height,depth,border,format,type,pixels){if(GLctx.currentPixelUnpackBufferBinding){GLctx["texImage3D"](target,level,internalFormat,width,height,depth,border,format,type,pixels)}else if(pixels){var heap=heapObjectForWebGLType(type);GLctx["texImage3D"](target,level,internalFormat,width,height,depth,border,format,type,heap,pixels>>heapAccessShiftForWebGLHeap(heap))}else{GLctx["texImage3D"](target,level,internalFormat,width,height,depth,border,format,type,null)}}function _emscripten_glTexParameterf(x0,x1,x2){GLctx["texParameterf"](x0,x1,x2)}function _emscripten_glTexParameterfv(target,pname,params){var param=HEAPF32[params>>2];GLctx.texParameterf(target,pname,param)}function _emscripten_glTexParameteri(x0,x1,x2){GLctx["texParameteri"](x0,x1,x2)}function _emscripten_glTexParameteriv(target,pname,params){var param=HEAP32[params>>2];GLctx.texParameteri(target,pname,param)}function _emscripten_glTexStorage2D(x0,x1,x2,x3,x4){GLctx["texStorage2D"](x0,x1,x2,x3,x4)}function _emscripten_glTexStorage3D(x0,x1,x2,x3,x4,x5){GLctx["texStorage3D"](x0,x1,x2,x3,x4,x5)}function _emscripten_glTexSubImage2D(target,level,xoffset,yoffset,width,height,format,type,pixels){if(true){if(GLctx.currentPixelUnpackBufferBinding){GLctx.texSubImage2D(target,level,xoffset,yoffset,width,height,format,type,pixels)}else if(pixels){var heap=heapObjectForWebGLType(type);GLctx.texSubImage2D(target,level,xoffset,yoffset,width,height,format,type,heap,pixels>>heapAccessShiftForWebGLHeap(heap))}else{GLctx.texSubImage2D(target,level,xoffset,yoffset,width,height,format,type,null)}return}var pixelData=null;if(pixels)pixelData=emscriptenWebGLGetTexPixelData(type,format,width,height,pixels,0);GLctx.texSubImage2D(target,level,xoffset,yoffset,width,height,format,type,pixelData)}function _emscripten_glTexSubImage3D(target,level,xoffset,yoffset,zoffset,width,height,depth,format,type,pixels){if(GLctx.currentPixelUnpackBufferBinding){GLctx["texSubImage3D"](target,level,xoffset,yoffset,zoffset,width,height,depth,format,type,pixels)}else if(pixels){var heap=heapObjectForWebGLType(type);GLctx["texSubImage3D"](target,level,xoffset,yoffset,zoffset,width,height,depth,format,type,heap,pixels>>heapAccessShiftForWebGLHeap(heap))}else{GLctx["texSubImage3D"](target,level,xoffset,yoffset,zoffset,width,height,depth,format,type,null)}}function _emscripten_glTransformFeedbackVaryings(program,count,varyings,bufferMode){program=GL.programs[program];var vars=[];for(var i=0;i<count;i++)vars.push(UTF8ToString(HEAP32[varyings+i*4>>2]));GLctx["transformFeedbackVaryings"](program,vars,bufferMode)}function _emscripten_glUniform1f(location,v0){GLctx.uniform1f(GL.uniforms[location],v0)}function _emscripten_glUniform1fv(location,count,value){GLctx.uniform1fv(GL.uniforms[location],HEAPF32,value>>2,count)}function _emscripten_glUniform1i(location,v0){GLctx.uniform1i(GL.uniforms[location],v0)}function _emscripten_glUniform1iv(location,count,value){GLctx.uniform1iv(GL.uniforms[location],HEAP32,value>>2,count)}function _emscripten_glUniform1ui(location,v0){GLctx.uniform1ui(GL.uniforms[location],v0)}function _emscripten_glUniform1uiv(location,count,value){GLctx.uniform1uiv(GL.uniforms[location],HEAPU32,value>>2,count)}function _emscripten_glUniform2f(location,v0,v1){GLctx.uniform2f(GL.uniforms[location],v0,v1)}function _emscripten_glUniform2fv(location,count,value){GLctx.uniform2fv(GL.uniforms[location],HEAPF32,value>>2,count*2)}function _emscripten_glUniform2i(location,v0,v1){GLctx.uniform2i(GL.uniforms[location],v0,v1)}function _emscripten_glUniform2iv(location,count,value){GLctx.uniform2iv(GL.uniforms[location],HEAP32,value>>2,count*2)}function _emscripten_glUniform2ui(location,v0,v1){GLctx.uniform2ui(GL.uniforms[location],v0,v1)}function _emscripten_glUniform2uiv(location,count,value){GLctx.uniform2uiv(GL.uniforms[location],HEAPU32,value>>2,count*2)}function _emscripten_glUniform3f(location,v0,v1,v2){GLctx.uniform3f(GL.uniforms[location],v0,v1,v2)}function _emscripten_glUniform3fv(location,count,value){GLctx.uniform3fv(GL.uniforms[location],HEAPF32,value>>2,count*3)}function _emscripten_glUniform3i(location,v0,v1,v2){GLctx.uniform3i(GL.uniforms[location],v0,v1,v2)}function _emscripten_glUniform3iv(location,count,value){GLctx.uniform3iv(GL.uniforms[location],HEAP32,value>>2,count*3)}function _emscripten_glUniform3ui(location,v0,v1,v2){GLctx.uniform3ui(GL.uniforms[location],v0,v1,v2)}function _emscripten_glUniform3uiv(location,count,value){GLctx.uniform3uiv(GL.uniforms[location],HEAPU32,value>>2,count*3)}function _emscripten_glUniform4f(location,v0,v1,v2,v3){GLctx.uniform4f(GL.uniforms[location],v0,v1,v2,v3)}function _emscripten_glUniform4fv(location,count,value){GLctx.uniform4fv(GL.uniforms[location],HEAPF32,value>>2,count*4)}function _emscripten_glUniform4i(location,v0,v1,v2,v3){GLctx.uniform4i(GL.uniforms[location],v0,v1,v2,v3)}function _emscripten_glUniform4iv(location,count,value){GLctx.uniform4iv(GL.uniforms[location],HEAP32,value>>2,count*4)}function _emscripten_glUniform4ui(location,v0,v1,v2,v3){GLctx.uniform4ui(GL.uniforms[location],v0,v1,v2,v3)}function _emscripten_glUniform4uiv(location,count,value){GLctx.uniform4uiv(GL.uniforms[location],HEAPU32,value>>2,count*4)}function _emscripten_glUniformBlockBinding(program,uniformBlockIndex,uniformBlockBinding){program=GL.programs[program];GLctx["uniformBlockBinding"](program,uniformBlockIndex,uniformBlockBinding)}function _emscripten_glUniformMatrix2fv(location,count,transpose,value){GLctx.uniformMatrix2fv(GL.uniforms[location],!!transpose,HEAPF32,value>>2,count*4)}function _emscripten_glUniformMatrix2x3fv(location,count,transpose,value){GLctx.uniformMatrix2x3fv(GL.uniforms[location],!!transpose,HEAPF32,value>>2,count*6)}function _emscripten_glUniformMatrix2x4fv(location,count,transpose,value){GLctx.uniformMatrix2x4fv(GL.uniforms[location],!!transpose,HEAPF32,value>>2,count*8)}function _emscripten_glUniformMatrix3fv(location,count,transpose,value){GLctx.uniformMatrix3fv(GL.uniforms[location],!!transpose,HEAPF32,value>>2,count*9)}function _emscripten_glUniformMatrix3x2fv(location,count,transpose,value){GLctx.uniformMatrix3x2fv(GL.uniforms[location],!!transpose,HEAPF32,value>>2,count*6)}function _emscripten_glUniformMatrix3x4fv(location,count,transpose,value){GLctx.uniformMatrix3x4fv(GL.uniforms[location],!!transpose,HEAPF32,value>>2,count*12)}function _emscripten_glUniformMatrix4fv(location,count,transpose,value){GLctx.uniformMatrix4fv(GL.uniforms[location],!!transpose,HEAPF32,value>>2,count*16)}function _emscripten_glUniformMatrix4x2fv(location,count,transpose,value){GLctx.uniformMatrix4x2fv(GL.uniforms[location],!!transpose,HEAPF32,value>>2,count*8)}function _emscripten_glUniformMatrix4x3fv(location,count,transpose,value){GLctx.uniformMatrix4x3fv(GL.uniforms[location],!!transpose,HEAPF32,value>>2,count*12)}function _emscripten_glUnmapBuffer(target){if(!emscriptenWebGLValidateMapBufferTarget(target)){GL.recordError(1280);err("GL_INVALID_ENUM in glUnmapBuffer");return 0}var buffer=emscriptenWebGLGetBufferBinding(target);var mapping=GL.mappedBuffers[buffer];if(!mapping){GL.recordError(1282);err("buffer was never mapped in glUnmapBuffer");return 0}GL.mappedBuffers[buffer]=null;if(!(mapping.access&16))if(true){GLctx.bufferSubData(target,mapping.offset,HEAPU8,mapping.mem,mapping.length)}else{GLctx.bufferSubData(target,mapping.offset,HEAPU8.subarray(mapping.mem,mapping.mem+mapping.length))}_free(mapping.mem);return 1}function _emscripten_glUseProgram(program){GLctx.useProgram(GL.programs[program])}function _emscripten_glValidateProgram(program){GLctx.validateProgram(GL.programs[program])}function _emscripten_glVertexAttrib1f(x0,x1){GLctx["vertexAttrib1f"](x0,x1)}function _emscripten_glVertexAttrib1fv(index,v){GLctx.vertexAttrib1f(index,HEAPF32[v>>2])}function _emscripten_glVertexAttrib2f(x0,x1,x2){GLctx["vertexAttrib2f"](x0,x1,x2)}function _emscripten_glVertexAttrib2fv(index,v){GLctx.vertexAttrib2f(index,HEAPF32[v>>2],HEAPF32[v+4>>2])}function _emscripten_glVertexAttrib3f(x0,x1,x2,x3){GLctx["vertexAttrib3f"](x0,x1,x2,x3)}function _emscripten_glVertexAttrib3fv(index,v){GLctx.vertexAttrib3f(index,HEAPF32[v>>2],HEAPF32[v+4>>2],HEAPF32[v+8>>2])}function _emscripten_glVertexAttrib4f(x0,x1,x2,x3,x4){GLctx["vertexAttrib4f"](x0,x1,x2,x3,x4)}function _emscripten_glVertexAttrib4fv(index,v){GLctx.vertexAttrib4f(index,HEAPF32[v>>2],HEAPF32[v+4>>2],HEAPF32[v+8>>2],HEAPF32[v+12>>2])}function _emscripten_glVertexAttribDivisor(index,divisor){GLctx["vertexAttribDivisor"](index,divisor)}function _emscripten_glVertexAttribDivisorANGLE(index,divisor){GLctx["vertexAttribDivisor"](index,divisor)}function _emscripten_glVertexAttribDivisorARB(index,divisor){GLctx["vertexAttribDivisor"](index,divisor)}function _emscripten_glVertexAttribDivisorEXT(index,divisor){GLctx["vertexAttribDivisor"](index,divisor)}function _emscripten_glVertexAttribDivisorNV(index,divisor){GLctx["vertexAttribDivisor"](index,divisor)}function _emscripten_glVertexAttribI4i(x0,x1,x2,x3,x4){GLctx["vertexAttribI4i"](x0,x1,x2,x3,x4)}function _emscripten_glVertexAttribI4iv(index,v){GLctx.vertexAttribI4i(index,HEAP32[v>>2],HEAP32[v+4>>2],HEAP32[v+8>>2],HEAP32[v+12>>2])}function _emscripten_glVertexAttribI4ui(x0,x1,x2,x3,x4){GLctx["vertexAttribI4ui"](x0,x1,x2,x3,x4)}function _emscripten_glVertexAttribI4uiv(index,v){GLctx.vertexAttribI4ui(index,HEAPU32[v>>2],HEAPU32[v+4>>2],HEAPU32[v+8>>2],HEAPU32[v+12>>2])}function _emscripten_glVertexAttribIPointer(index,size,type,stride,ptr){var cb=GL.currentContext.clientBuffers[index];if(!GLctx.currentArrayBufferBinding){cb.size=size;cb.type=type;cb.normalized=false;cb.stride=stride;cb.ptr=ptr;cb.clientside=true;cb.vertexAttribPointerAdaptor=function(index,size,type,normalized,stride,ptr){this.vertexAttribIPointer(index,size,type,stride,ptr)};return}cb.clientside=false;GLctx["vertexAttribIPointer"](index,size,type,stride,ptr)}function _emscripten_glVertexAttribPointer(index,size,type,normalized,stride,ptr){var cb=GL.currentContext.clientBuffers[index];if(!GLctx.currentArrayBufferBinding){cb.size=size;cb.type=type;cb.normalized=normalized;cb.stride=stride;cb.ptr=ptr;cb.clientside=true;cb.vertexAttribPointerAdaptor=function(index,size,type,normalized,stride,ptr){this.vertexAttribPointer(index,size,type,normalized,stride,ptr)};return}cb.clientside=false;GLctx.vertexAttribPointer(index,size,type,!!normalized,stride,ptr)}function _emscripten_glViewport(x0,x1,x2,x3){GLctx["viewport"](x0,x1,x2,x3)}function _emscripten_glWaitSync(sync,flags,timeoutLo,timeoutHi){GLctx.waitSync(GL.syncs[sync],flags,convertI32PairToI53(timeoutLo,timeoutHi))}function _emscripten_memcpy_big(dest,src,num){HEAPU8.copyWithin(dest,src,src+num)}function _emscripten_get_heap_size(){return HEAPU8.length}function emscripten_realloc_buffer(size){try{wasmMemory.grow(size-buffer.byteLength+65535>>>16);updateGlobalBufferAndViews(wasmMemory.buffer);return 1}catch(e){}}function _emscripten_resize_heap(requestedSize){requestedSize=requestedSize>>>0;var oldSize=_emscripten_get_heap_size();var maxHeapSize=2147483648;if(requestedSize>maxHeapSize){return false}var minHeapSize=16777216;for(var cutDown=1;cutDown<=4;cutDown*=2){var overGrownHeapSize=oldSize*(1+.2/cutDown);overGrownHeapSize=Math.min(overGrownHeapSize,requestedSize+100663296);var newSize=Math.min(maxHeapSize,alignUp(Math.max(minHeapSize,requestedSize,overGrownHeapSize),65536));var replacement=emscripten_realloc_buffer(newSize);if(replacement){return true}}return false}var SYSCALLS={mappings:{},buffers:[null,[],[]],printChar:function(stream,curr){var buffer=SYSCALLS.buffers[stream];if(curr===0||curr===10){(stream===1?out:err)(UTF8ArrayToString(buffer,0));buffer.length=0}else{buffer.push(curr)}},varargs:undefined,get:function(){SYSCALLS.varargs+=4;var ret=HEAP32[SYSCALLS.varargs-4>>2];return ret},getStr:function(ptr){var ret=UTF8ToString(ptr);return ret},get64:function(low,high){return low}};function _fd_close(fd){return 0}function _fd_seek(fd,offset_low,offset_high,whence,newOffset){}function _fd_write(fd,iov,iovcnt,pnum){var num=0;for(var i=0;i<iovcnt;i++){var ptr=HEAP32[iov+i*8>>2];var len=HEAP32[iov+(i*8+4)>>2];for(var j=0;j<len;j++){SYSCALLS.printChar(fd,HEAPU8[ptr+j])}num+=len}HEAP32[pnum>>2]=num;return 0}function getRandomDevice(){if(typeof crypto==="object"&&typeof crypto["getRandomValues"]==="function"){var randomBuffer=new Uint8Array(1);return function(){crypto.getRandomValues(randomBuffer);return randomBuffer[0]}}else if(ENVIRONMENT_IS_NODE){try{var crypto_module=require("crypto");return function(){return crypto_module["randomBytes"](1)[0]}}catch(e){}}return function(){abort("randomDevice")}}function _getentropy(buffer,size){if(!_getentropy.randomDevice){_getentropy.randomDevice=getRandomDevice()}for(var i=0;i<size;i++){HEAP8[buffer+i>>0]=_getentropy.randomDevice()}return 0}function _glActiveTexture(x0){GLctx["activeTexture"](x0)}function _glAttachShader(program,shader){GLctx.attachShader(GL.programs[program],GL.shaders[shader])}function _glBeginQuery(target,id){GLctx["beginQuery"](target,GL.queries[id])}function _glBindBuffer(target,buffer){if(target==34962){GLctx.currentArrayBufferBinding=buffer}else if(target==34963){GLctx.currentElementArrayBufferBinding=buffer}if(target==35051){GLctx.currentPixelPackBufferBinding=buffer}else if(target==35052){GLctx.currentPixelUnpackBufferBinding=buffer}GLctx.bindBuffer(target,GL.buffers[buffer])}function _glBindBufferRange(target,index,buffer,offset,ptrsize){GLctx["bindBufferRange"](target,index,GL.buffers[buffer],offset,ptrsize)}function _glBindFramebuffer(target,framebuffer){GLctx.bindFramebuffer(target,GL.framebuffers[framebuffer])}function _glBindRenderbuffer(target,renderbuffer){GLctx.bindRenderbuffer(target,GL.renderbuffers[renderbuffer])}function _glBindSampler(unit,sampler){GLctx["bindSampler"](unit,GL.samplers[sampler])}function _glBindTexture(target,texture){GLctx.bindTexture(target,GL.textures[texture])}function _glBindVertexArray(vao){GLctx["bindVertexArray"](GL.vaos[vao]);var ibo=GLctx.getParameter(34965);GLctx.currentElementArrayBufferBinding=ibo?ibo.name|0:0}function _glBlendEquationSeparate(x0,x1){GLctx["blendEquationSeparate"](x0,x1)}function _glBlendFuncSeparate(x0,x1,x2,x3){GLctx["blendFuncSeparate"](x0,x1,x2,x3)}function _glBlitFramebuffer(x0,x1,x2,x3,x4,x5,x6,x7,x8,x9){GLctx["blitFramebuffer"](x0,x1,x2,x3,x4,x5,x6,x7,x8,x9)}function _glBufferData(target,size,data,usage){if(true){if(data){GLctx.bufferData(target,HEAPU8,usage,data,size)}else{GLctx.bufferData(target,size,usage)}}else{GLctx.bufferData(target,data?HEAPU8.subarray(data,data+size):size,usage)}}function _glBufferSubData(target,offset,size,data){if(true){GLctx.bufferSubData(target,offset,HEAPU8,data,size);return}GLctx.bufferSubData(target,offset,HEAPU8.subarray(data,data+size))}function _glClearBufferfi(x0,x1,x2,x3){GLctx["clearBufferfi"](x0,x1,x2,x3)}function _glClearBufferfv(buffer,drawbuffer,value){GLctx["clearBufferfv"](buffer,drawbuffer,HEAPF32,value>>2)}function _glClearBufferiv(buffer,drawbuffer,value){GLctx["clearBufferiv"](buffer,drawbuffer,HEAP32,value>>2)}function _glClientWaitSync(sync,flags,timeoutLo,timeoutHi){return GLctx.clientWaitSync(GL.syncs[sync],flags,convertI32PairToI53(timeoutLo,timeoutHi))}function _glColorMask(red,green,blue,alpha){GLctx.colorMask(!!red,!!green,!!blue,!!alpha)}function _glCompileShader(shader){GLctx.compileShader(GL.shaders[shader])}function _glCompressedTexSubImage2D(target,level,xoffset,yoffset,width,height,format,imageSize,data){if(true){if(GLctx.currentPixelUnpackBufferBinding){GLctx["compressedTexSubImage2D"](target,level,xoffset,yoffset,width,height,format,imageSize,data)}else{GLctx["compressedTexSubImage2D"](target,level,xoffset,yoffset,width,height,format,HEAPU8,data,imageSize)}return}GLctx["compressedTexSubImage2D"](target,level,xoffset,yoffset,width,height,format,data?HEAPU8.subarray(data,data+imageSize):null)}function _glCompressedTexSubImage3D(target,level,xoffset,yoffset,zoffset,width,height,depth,format,imageSize,data){if(GLctx.currentPixelUnpackBufferBinding){GLctx["compressedTexSubImage3D"](target,level,xoffset,yoffset,zoffset,width,height,depth,format,imageSize,data)}else{GLctx["compressedTexSubImage3D"](target,level,xoffset,yoffset,zoffset,width,height,depth,format,HEAPU8,data,imageSize)}}function _glCreateProgram(){var id=GL.getNewId(GL.programs);var program=GLctx.createProgram();program.name=id;GL.programs[id]=program;return id}function _glCreateShader(shaderType){var id=GL.getNewId(GL.shaders);GL.shaders[id]=GLctx.createShader(shaderType);return id}function _glCullFace(x0){GLctx["cullFace"](x0)}function _glDeleteBuffers(n,buffers){for(var i=0;i<n;i++){var id=HEAP32[buffers+i*4>>2];var buffer=GL.buffers[id];if(!buffer)continue;GLctx.deleteBuffer(buffer);buffer.name=0;GL.buffers[id]=null;if(id==GLctx.currentArrayBufferBinding)GLctx.currentArrayBufferBinding=0;if(id==GLctx.currentElementArrayBufferBinding)GLctx.currentElementArrayBufferBinding=0;if(id==GLctx.currentPixelPackBufferBinding)GLctx.currentPixelPackBufferBinding=0;if(id==GLctx.currentPixelUnpackBufferBinding)GLctx.currentPixelUnpackBufferBinding=0}}function _glDeleteFramebuffers(n,framebuffers){for(var i=0;i<n;++i){var id=HEAP32[framebuffers+i*4>>2];var framebuffer=GL.framebuffers[id];if(!framebuffer)continue;GLctx.deleteFramebuffer(framebuffer);framebuffer.name=0;GL.framebuffers[id]=null}}function _glDeleteProgram(id){if(!id)return;var program=GL.programs[id];if(!program){GL.recordError(1281);return}GLctx.deleteProgram(program);program.name=0;GL.programs[id]=null;GL.programInfos[id]=null}function _glDeleteQueries(n,ids){for(var i=0;i<n;i++){var id=HEAP32[ids+i*4>>2];var query=GL.queries[id];if(!query)continue;GLctx["deleteQuery"](query);GL.queries[id]=null}}function _glDeleteRenderbuffers(n,renderbuffers){for(var i=0;i<n;i++){var id=HEAP32[renderbuffers+i*4>>2];var renderbuffer=GL.renderbuffers[id];if(!renderbuffer)continue;GLctx.deleteRenderbuffer(renderbuffer);renderbuffer.name=0;GL.renderbuffers[id]=null}}function _glDeleteSamplers(n,samplers){for(var i=0;i<n;i++){var id=HEAP32[samplers+i*4>>2];var sampler=GL.samplers[id];if(!sampler)continue;GLctx["deleteSampler"](sampler);sampler.name=0;GL.samplers[id]=null}}function _glDeleteShader(id){if(!id)return;var shader=GL.shaders[id];if(!shader){GL.recordError(1281);return}GLctx.deleteShader(shader);GL.shaders[id]=null}function _glDeleteSync(id){if(!id)return;var sync=GL.syncs[id];if(!sync){GL.recordError(1281);return}GLctx.deleteSync(sync);sync.name=0;GL.syncs[id]=null}function _glDeleteTextures(n,textures){for(var i=0;i<n;i++){var id=HEAP32[textures+i*4>>2];var texture=GL.textures[id];if(!texture)continue;GLctx.deleteTexture(texture);texture.name=0;GL.textures[id]=null}}function _glDeleteVertexArrays(n,vaos){for(var i=0;i<n;i++){var id=HEAP32[vaos+i*4>>2];GLctx["deleteVertexArray"](GL.vaos[id]);GL.vaos[id]=null}}function _glDepthFunc(x0){GLctx["depthFunc"](x0)}function _glDepthMask(flag){GLctx.depthMask(!!flag)}function _glDepthRangef(x0,x1){GLctx["depthRange"](x0,x1)}function _glDetachShader(program,shader){GLctx.detachShader(GL.programs[program],GL.shaders[shader])}function _glDisable(x0){GLctx["disable"](x0)}function _glDisableVertexAttribArray(index){var cb=GL.currentContext.clientBuffers[index];cb.enabled=false;GLctx.disableVertexAttribArray(index)}function _glDrawArrays(mode,first,count){GL.preDrawHandleClientVertexAttribBindings(first+count);GLctx.drawArrays(mode,first,count);GL.postDrawHandleClientVertexAttribBindings()}function _glDrawBuffers(n,bufs){var bufArray=tempFixedLengthArray[n];for(var i=0;i<n;i++){bufArray[i]=HEAP32[bufs+i*4>>2]}GLctx["drawBuffers"](bufArray)}function _glDrawRangeElements(mode,start,end,count,type,indices){_glDrawElements(mode,count,type,indices)}function _glEnable(x0){GLctx["enable"](x0)}function _glEnableVertexAttribArray(index){var cb=GL.currentContext.clientBuffers[index];cb.enabled=true;GLctx.enableVertexAttribArray(index)}function _glEndQuery(x0){GLctx["endQuery"](x0)}function _glFenceSync(condition,flags){var sync=GLctx.fenceSync(condition,flags);if(sync){var id=GL.getNewId(GL.syncs);sync.name=id;GL.syncs[id]=sync;return id}else{return 0}}function _glFinish(){GLctx["finish"]()}function _glFlush(){GLctx["flush"]()}function _glFramebufferRenderbuffer(target,attachment,renderbuffertarget,renderbuffer){GLctx.framebufferRenderbuffer(target,attachment,renderbuffertarget,GL.renderbuffers[renderbuffer])}function _glFramebufferTexture2D(target,attachment,textarget,texture,level){GLctx.framebufferTexture2D(target,attachment,textarget,GL.textures[texture],level)}function _glFramebufferTextureLayer(target,attachment,texture,level,layer){GLctx.framebufferTextureLayer(target,attachment,GL.textures[texture],level,layer)}function _glFrontFace(x0){GLctx["frontFace"](x0)}function _glGenBuffers(n,buffers){__glGenObject(n,buffers,"createBuffer",GL.buffers)}function _glGenFramebuffers(n,ids){__glGenObject(n,ids,"createFramebuffer",GL.framebuffers)}function _glGenQueries(n,ids){__glGenObject(n,ids,"createQuery",GL.queries)}function _glGenRenderbuffers(n,renderbuffers){__glGenObject(n,renderbuffers,"createRenderbuffer",GL.renderbuffers)}function _glGenSamplers(n,samplers){__glGenObject(n,samplers,"createSampler",GL.samplers)}function _glGenTextures(n,textures){__glGenObject(n,textures,"createTexture",GL.textures)}function _glGenVertexArrays(n,arrays){__glGenObject(n,arrays,"createVertexArray",GL.vaos)}function _glGenerateMipmap(x0){GLctx["generateMipmap"](x0)}function _glGetError(){var error=GLctx.getError()||GL.lastError;GL.lastError=0;return error}function _glGetFloatv(name_,p){emscriptenWebGLGet(name_,p,2)}function _glGetIntegerv(name_,p){emscriptenWebGLGet(name_,p,0)}function _glGetProgramInfoLog(program,maxLength,length,infoLog){var log=GLctx.getProgramInfoLog(GL.programs[program]);if(log===null)log="(unknown error)";var numBytesWrittenExclNull=maxLength>0&&infoLog?stringToUTF8(log,infoLog,maxLength):0;if(length)HEAP32[length>>2]=numBytesWrittenExclNull}function _glGetProgramiv(program,pname,p){if(!p){GL.recordError(1281);return}if(program>=GL.counter){GL.recordError(1281);return}var ptable=GL.programInfos[program];if(!ptable){GL.recordError(1282);return}if(pname==35716){var log=GLctx.getProgramInfoLog(GL.programs[program]);if(log===null)log="(unknown error)";HEAP32[p>>2]=log.length+1}else if(pname==35719){HEAP32[p>>2]=ptable.maxUniformLength}else if(pname==35722){if(ptable.maxAttributeLength==-1){program=GL.programs[program];var numAttribs=GLctx.getProgramParameter(program,35721);ptable.maxAttributeLength=0;for(var i=0;i<numAttribs;++i){var activeAttrib=GLctx.getActiveAttrib(program,i);ptable.maxAttributeLength=Math.max(ptable.maxAttributeLength,activeAttrib.name.length+1)}}HEAP32[p>>2]=ptable.maxAttributeLength}else if(pname==35381){if(ptable.maxUniformBlockNameLength==-1){program=GL.programs[program];var numBlocks=GLctx.getProgramParameter(program,35382);ptable.maxUniformBlockNameLength=0;for(var i=0;i<numBlocks;++i){var activeBlockName=GLctx.getActiveUniformBlockName(program,i);ptable.maxUniformBlockNameLength=Math.max(ptable.maxUniformBlockNameLength,activeBlockName.length+1)}}HEAP32[p>>2]=ptable.maxUniformBlockNameLength}else{HEAP32[p>>2]=GLctx.getProgramParameter(GL.programs[program],pname)}}function _glGetQueryObjectuiv(id,pname,params){if(!params){GL.recordError(1281);return}var query=GL.queries[id];var param=GLctx["getQueryParameter"](query,pname);var ret;if(typeof param=="boolean"){ret=param?1:0}else{ret=param}HEAP32[params>>2]=ret}function _glGetShaderInfoLog(shader,maxLength,length,infoLog){var log=GLctx.getShaderInfoLog(GL.shaders[shader]);if(log===null)log="(unknown error)";var numBytesWrittenExclNull=maxLength>0&&infoLog?stringToUTF8(log,infoLog,maxLength):0;if(length)HEAP32[length>>2]=numBytesWrittenExclNull}function _glGetShaderiv(shader,pname,p){if(!p){GL.recordError(1281);return}if(pname==35716){var log=GLctx.getShaderInfoLog(GL.shaders[shader]);if(log===null)log="(unknown error)";var logLength=log?log.length+1:0;HEAP32[p>>2]=logLength}else if(pname==35720){var source=GLctx.getShaderSource(GL.shaders[shader]);var sourceLength=source?source.length+1:0;HEAP32[p>>2]=sourceLength}else{HEAP32[p>>2]=GLctx.getShaderParameter(GL.shaders[shader],pname)}}function _glGetString(name_){if(GL.stringCache[name_])return GL.stringCache[name_];var ret;switch(name_){case 7939:var exts=GLctx.getSupportedExtensions()||[];exts=exts.concat(exts.map(function(e){return"GL_"+e}));ret=stringToNewUTF8(exts.join(" "));break;case 7936:case 7937:case 37445:case 37446:var s=GLctx.getParameter(name_);if(!s){GL.recordError(1280)}ret=stringToNewUTF8(s);break;case 7938:var glVersion=GLctx.getParameter(7938);if(true)glVersion="OpenGL ES 3.0 ("+glVersion+")";else{glVersion="OpenGL ES 2.0 ("+glVersion+")"}ret=stringToNewUTF8(glVersion);break;case 35724:var glslVersion=GLctx.getParameter(35724);var ver_re=/^WebGL GLSL ES ([0-9]\.[0-9][0-9]?)(?:$| .*)/;var ver_num=glslVersion.match(ver_re);if(ver_num!==null){if(ver_num[1].length==3)ver_num[1]=ver_num[1]+"0";glslVersion="OpenGL ES GLSL ES "+ver_num[1]+" ("+glslVersion+")"}ret=stringToNewUTF8(glslVersion);break;default:GL.recordError(1280);return 0}GL.stringCache[name_]=ret;return ret}function _glGetStringi(name,index){if(GL.currentContext.version<2){GL.recordError(1282);return 0}var stringiCache=GL.stringiCache[name];if(stringiCache){if(index<0||index>=stringiCache.length){GL.recordError(1281);return 0}return stringiCache[index]}switch(name){case 7939:var exts=GLctx.getSupportedExtensions()||[];exts=exts.concat(exts.map(function(e){return"GL_"+e}));exts=exts.map(function(e){return stringToNewUTF8(e)});stringiCache=GL.stringiCache[name]=exts;if(index<0||index>=stringiCache.length){GL.recordError(1281);return 0}return stringiCache[index];default:GL.recordError(1280);return 0}}function _glGetUniformBlockIndex(program,uniformBlockName){return GLctx["getUniformBlockIndex"](GL.programs[program],UTF8ToString(uniformBlockName))}function _glGetUniformLocation(program,name){name=UTF8ToString(name);var arrayIndex=0;if(name[name.length-1]=="]"){var leftBrace=name.lastIndexOf("[");arrayIndex=name[leftBrace+1]!="]"?jstoi_q(name.slice(leftBrace+1)):0;name=name.slice(0,leftBrace)}var uniformInfo=GL.programInfos[program]&&GL.programInfos[program].uniforms[name];if(uniformInfo&&arrayIndex>=0&&arrayIndex<uniformInfo[0]){return uniformInfo[1]+arrayIndex}else{return-1}}function _glGetVertexAttribiv(index,pname,params){emscriptenWebGLGetVertexAttrib(index,pname,params,5)}function _glHint(x0,x1){GLctx["hint"](x0,x1)}function _glInvalidateFramebuffer(target,numAttachments,attachments){var list=tempFixedLengthArray[numAttachments];for(var i=0;i<numAttachments;i++){list[i]=HEAP32[attachments+i*4>>2]}GLctx["invalidateFramebuffer"](target,list)}function _glIsEnabled(x0){return GLctx["isEnabled"](x0)}function _glLinkProgram(program){GLctx.linkProgram(GL.programs[program]);GL.populateUniformTable(program)}function _glMapBufferRange(target,offset,length,access){if(access!=26&&access!=10){err("glMapBufferRange is only supported when access is MAP_WRITE|INVALIDATE_BUFFER");return 0}if(!emscriptenWebGLValidateMapBufferTarget(target)){GL.recordError(1280);err("GL_INVALID_ENUM in glMapBufferRange");return 0}var mem=_malloc(length);if(!mem)return 0;GL.mappedBuffers[emscriptenWebGLGetBufferBinding(target)]={offset:offset,length:length,mem:mem,access:access};return mem}function _glPixelStorei(pname,param){if(pname==3317){GL.unpackAlignment=param}GLctx.pixelStorei(pname,param)}function _glPolygonOffset(x0,x1){GLctx["polygonOffset"](x0,x1)}function _glReadPixels(x,y,width,height,format,type,pixels){if(true){if(GLctx.currentPixelPackBufferBinding){GLctx.readPixels(x,y,width,height,format,type,pixels)}else{var heap=heapObjectForWebGLType(type);GLctx.readPixels(x,y,width,height,format,type,heap,pixels>>heapAccessShiftForWebGLHeap(heap))}return}var pixelData=emscriptenWebGLGetTexPixelData(type,format,width,height,pixels,format);if(!pixelData){GL.recordError(1280);return}GLctx.readPixels(x,y,width,height,format,type,pixelData)}function _glRenderbufferStorage(x0,x1,x2,x3){GLctx["renderbufferStorage"](x0,x1,x2,x3)}function _glRenderbufferStorageMultisample(x0,x1,x2,x3,x4){GLctx["renderbufferStorageMultisample"](x0,x1,x2,x3,x4)}function _glSamplerParameteri(sampler,pname,param){GLctx["samplerParameteri"](GL.samplers[sampler],pname,param)}function _glScissor(x0,x1,x2,x3){GLctx["scissor"](x0,x1,x2,x3)}function _glShaderSource(shader,count,string,length){var source=GL.getSource(shader,count,string,length);GLctx.shaderSource(GL.shaders[shader],source)}function _glTexParameteri(x0,x1,x2){GLctx["texParameteri"](x0,x1,x2)}function _glTexStorage2D(x0,x1,x2,x3,x4){GLctx["texStorage2D"](x0,x1,x2,x3,x4)}function _glTexStorage3D(x0,x1,x2,x3,x4,x5){GLctx["texStorage3D"](x0,x1,x2,x3,x4,x5)}function _glTexSubImage2D(target,level,xoffset,yoffset,width,height,format,type,pixels){if(true){if(GLctx.currentPixelUnpackBufferBinding){GLctx.texSubImage2D(target,level,xoffset,yoffset,width,height,format,type,pixels)}else if(pixels){var heap=heapObjectForWebGLType(type);GLctx.texSubImage2D(target,level,xoffset,yoffset,width,height,format,type,heap,pixels>>heapAccessShiftForWebGLHeap(heap))}else{GLctx.texSubImage2D(target,level,xoffset,yoffset,width,height,format,type,null)}return}var pixelData=null;if(pixels)pixelData=emscriptenWebGLGetTexPixelData(type,format,width,height,pixels,0);GLctx.texSubImage2D(target,level,xoffset,yoffset,width,height,format,type,pixelData)}function _glTexSubImage3D(target,level,xoffset,yoffset,zoffset,width,height,depth,format,type,pixels){if(GLctx.currentPixelUnpackBufferBinding){GLctx["texSubImage3D"](target,level,xoffset,yoffset,zoffset,width,height,depth,format,type,pixels)}else if(pixels){var heap=heapObjectForWebGLType(type);GLctx["texSubImage3D"](target,level,xoffset,yoffset,zoffset,width,height,depth,format,type,heap,pixels>>heapAccessShiftForWebGLHeap(heap))}else{GLctx["texSubImage3D"](target,level,xoffset,yoffset,zoffset,width,height,depth,format,type,null)}}function _glUniform1i(location,v0){GLctx.uniform1i(GL.uniforms[location],v0)}function _glUniformBlockBinding(program,uniformBlockIndex,uniformBlockBinding){program=GL.programs[program];GLctx["uniformBlockBinding"](program,uniformBlockIndex,uniformBlockBinding)}function _glUnmapBuffer(target){if(!emscriptenWebGLValidateMapBufferTarget(target)){GL.recordError(1280);err("GL_INVALID_ENUM in glUnmapBuffer");return 0}var buffer=emscriptenWebGLGetBufferBinding(target);var mapping=GL.mappedBuffers[buffer];if(!mapping){GL.recordError(1282);err("buffer was never mapped in glUnmapBuffer");return 0}GL.mappedBuffers[buffer]=null;if(!(mapping.access&16))if(true){GLctx.bufferSubData(target,mapping.offset,HEAPU8,mapping.mem,mapping.length)}else{GLctx.bufferSubData(target,mapping.offset,HEAPU8.subarray(mapping.mem,mapping.mem+mapping.length))}_free(mapping.mem);return 1}function _glUseProgram(program){GLctx.useProgram(GL.programs[program])}function _glVertexAttrib4f(x0,x1,x2,x3,x4){GLctx["vertexAttrib4f"](x0,x1,x2,x3,x4)}function _glVertexAttribI4ui(x0,x1,x2,x3,x4){GLctx["vertexAttribI4ui"](x0,x1,x2,x3,x4)}function _glVertexAttribIPointer(index,size,type,stride,ptr){var cb=GL.currentContext.clientBuffers[index];if(!GLctx.currentArrayBufferBinding){cb.size=size;cb.type=type;cb.normalized=false;cb.stride=stride;cb.ptr=ptr;cb.clientside=true;cb.vertexAttribPointerAdaptor=function(index,size,type,normalized,stride,ptr){this.vertexAttribIPointer(index,size,type,stride,ptr)};return}cb.clientside=false;GLctx["vertexAttribIPointer"](index,size,type,stride,ptr)}function _glVertexAttribPointer(index,size,type,normalized,stride,ptr){var cb=GL.currentContext.clientBuffers[index];if(!GLctx.currentArrayBufferBinding){cb.size=size;cb.type=type;cb.normalized=normalized;cb.stride=stride;cb.ptr=ptr;cb.clientside=true;cb.vertexAttribPointerAdaptor=function(index,size,type,normalized,stride,ptr){this.vertexAttribPointer(index,size,type,normalized,stride,ptr)};return}cb.clientside=false;GLctx.vertexAttribPointer(index,size,type,!!normalized,stride,ptr)}function _glViewport(x0,x1,x2,x3){GLctx["viewport"](x0,x1,x2,x3)}function _glWaitSync(sync,flags,timeoutLo,timeoutHi){GLctx.waitSync(GL.syncs[sync],flags,convertI32PairToI53(timeoutLo,timeoutHi))}function _pthread_create(){return 6}function _pthread_join(){return 28}function _setTempRet0($i){setTempRet0($i|0)}function _sysconf(name){switch(name){case 30:return 16384;case 85:var maxHeapSize=2147483648;return maxHeapSize/16384;case 132:case 133:case 12:case 137:case 138:case 15:case 235:case 16:case 17:case 18:case 19:case 20:case 149:case 13:case 10:case 236:case 153:case 9:case 21:case 22:case 159:case 154:case 14:case 77:case 78:case 139:case 82:case 68:case 67:case 164:case 11:case 29:case 47:case 48:case 95:case 52:case 51:case 46:return 200809;case 27:case 246:case 127:case 128:case 23:case 24:case 160:case 161:case 181:case 182:case 242:case 183:case 184:case 243:case 244:case 245:case 165:case 178:case 179:case 49:case 50:case 168:case 169:case 175:case 170:case 171:case 172:case 97:case 76:case 32:case 173:case 35:case 80:case 81:case 79:return-1;case 176:case 177:case 7:case 155:case 8:case 157:case 125:case 126:case 92:case 93:case 129:case 130:case 131:case 94:case 91:return 1;case 74:case 60:case 69:case 70:case 4:return 1024;case 31:case 42:case 72:return 32;case 87:case 26:case 33:return 2147483647;case 34:case 1:return 47839;case 38:case 36:return 99;case 43:case 37:return 2048;case 0:return 2097152;case 3:return 65536;case 28:return 32768;case 44:return 32767;case 75:return 16384;case 39:return 1e3;case 89:return 700;case 71:return 256;case 40:return 255;case 2:return 100;case 180:return 64;case 25:return 20;case 5:return 16;case 6:return 6;case 73:return 4;case 84:{if(typeof navigator==="object")return navigator["hardwareConcurrency"]||1;return 1}}setErrNo(28);return-1}var readAsmConstArgsArray=[];function readAsmConstArgs(sigPtr,buf){readAsmConstArgsArray.length=0;var ch;buf>>=2;while(ch=HEAPU8[sigPtr++]){var double=ch<105;if(double&&buf&1)buf++;readAsmConstArgsArray.push(double?HEAPF64[buf++>>1]:HEAP32[buf]);++buf}return readAsmConstArgsArray}InternalError=Module["InternalError"]=extendError(Error,"InternalError");embind_init_charCodes();BindingError=Module["BindingError"]=extendError(Error,"BindingError");init_ClassHandle();init_RegisteredPointer();init_embind();UnboundTypeError=Module["UnboundTypeError"]=extendError(Error,"UnboundTypeError");init_emval();var GLctx;for(var i=0;i<32;++i)tempFixedLengthArray.push(new Array(i));var asmLibraryArg={"A":__embind_finalize_value_array,"x":__embind_finalize_value_object,"Ag":__embind_register_bool,"e":__embind_register_class,"m":__embind_register_class_class_function,"o":__embind_register_class_constructor,"a":__embind_register_class_function,"u":__embind_register_class_property,"zg":__embind_register_emval,"i":__embind_register_enum,"b":__embind_register_enum_value,"Ia":__embind_register_float,"Ka":__embind_register_function,"D":__embind_register_integer,"v":__embind_register_memory_view,"Ja":__embind_register_std_string,"ia":__embind_register_std_wstring,"C":__embind_register_value_array,"f":__embind_register_value_array_element,"y":__embind_register_value_object,"d":__embind_register_value_object_field,"Bg":__embind_register_void,"q":__emval_as,"g":__emval_decref,"r":__emval_get_property,"ha":__emval_incref,"G":__emval_new_cstring,"p":__emval_run_destructors,"s":__emval_take_value,"c":_abort,"vg":_clock_gettime,"Fa":_emscripten_asm_const_int,"$f":_emscripten_glActiveTexture,"_f":_emscripten_glAttachShader,"cd":_emscripten_glBeginQuery,"pg":_emscripten_glBeginQueryEXT,"Fc":_emscripten_glBeginTransformFeedback,"Zf":_emscripten_glBindAttribLocation,"Yf":_emscripten_glBindBuffer,"Cc":_emscripten_glBindBufferBase,"Dc":_emscripten_glBindBufferRange,"Xf":_emscripten_glBindFramebuffer,"Wf":_emscripten_glBindRenderbuffer,"Kb":_emscripten_glBindSampler,"Vf":_emscripten_glBindTexture,"Bb":_emscripten_glBindTransformFeedback,"Lc":_emscripten_glBindVertexArray,"hg":_emscripten_glBindVertexArrayOES,"Uf":_emscripten_glBlendColor,"Tf":_emscripten_glBlendEquation,"Sf":_emscripten_glBlendEquationSeparate,"Rf":_emscripten_glBlendFunc,"Qf":_emscripten_glBlendFuncSeparate,"Qc":_emscripten_glBlitFramebuffer,"Pf":_emscripten_glBufferData,"Of":_emscripten_glBufferSubData,"Nf":_emscripten_glCheckFramebufferStatus,"Mf":_emscripten_glClear,"fc":_emscripten_glClearBufferfi,"gc":_emscripten_glClearBufferfv,"ic":_emscripten_glClearBufferiv,"hc":_emscripten_glClearBufferuiv,"Lf":_emscripten_glClearColor,"Kf":_emscripten_glClearDepthf,"Jf":_emscripten_glClearStencil,"Tb":_emscripten_glClientWaitSync,"If":_emscripten_glColorMask,"Hf":_emscripten_glCompileShader,"Gf":_emscripten_glCompressedTexImage2D,"hd":_emscripten_glCompressedTexImage3D,"Ff":_emscripten_glCompressedTexSubImage2D,"gd":_emscripten_glCompressedTexSubImage3D,"dc":_emscripten_glCopyBufferSubData,"Ef":_emscripten_glCopyTexImage2D,"Df":_emscripten_glCopyTexSubImage2D,"id":_emscripten_glCopyTexSubImage3D,"Cf":_emscripten_glCreateProgram,"Bf":_emscripten_glCreateShader,"Af":_emscripten_glCullFace,"zf":_emscripten_glDeleteBuffers,"yf":_emscripten_glDeleteFramebuffers,"xf":_emscripten_glDeleteProgram,"ed":_emscripten_glDeleteQueries,"rg":_emscripten_glDeleteQueriesEXT,"wf":_emscripten_glDeleteRenderbuffers,"Mb":_emscripten_glDeleteSamplers,"vf":_emscripten_glDeleteShader,"Ub":_emscripten_glDeleteSync,"uf":_emscripten_glDeleteTextures,"Ab":_emscripten_glDeleteTransformFeedbacks,"Kc":_emscripten_glDeleteVertexArrays,"gg":_emscripten_glDeleteVertexArraysOES,"tf":_emscripten_glDepthFunc,"sf":_emscripten_glDepthMask,"rf":_emscripten_glDepthRangef,"qf":_emscripten_glDetachShader,"pf":_emscripten_glDisable,"of":_emscripten_glDisableVertexAttribArray,"nf":_emscripten_glDrawArrays,"Yb":_emscripten_glDrawArraysInstanced,"cg":_emscripten_glDrawArraysInstancedANGLE,"kb":_emscripten_glDrawArraysInstancedARB,"qd":_emscripten_glDrawArraysInstancedEXT,"lb":_emscripten_glDrawArraysInstancedNV,"Xc":_emscripten_glDrawBuffers,"od":_emscripten_glDrawBuffersEXT,"dg":_emscripten_glDrawBuffersWEBGL,"mf":_emscripten_glDrawElements,"Xb":_emscripten_glDrawElementsInstanced,"bg":_emscripten_glDrawElementsInstancedANGLE,"ib":_emscripten_glDrawElementsInstancedARB,"jb":_emscripten_glDrawElementsInstancedEXT,"pd":_emscripten_glDrawElementsInstancedNV,"md":_emscripten_glDrawRangeElements,"lf":_emscripten_glEnable,"kf":_emscripten_glEnableVertexAttribArray,"bd":_emscripten_glEndQuery,"og":_emscripten_glEndQueryEXT,"Ec":_emscripten_glEndTransformFeedback,"Wb":_emscripten_glFenceSync,"jf":_emscripten_glFinish,"hf":_emscripten_glFlush,"Mc":_emscripten_glFlushMappedBufferRange,"gf":_emscripten_glFramebufferRenderbuffer,"ff":_emscripten_glFramebufferTexture2D,"Oc":_emscripten_glFramebufferTextureLayer,"ef":_emscripten_glFrontFace,"df":_emscripten_glGenBuffers,"bf":_emscripten_glGenFramebuffers,"fd":_emscripten_glGenQueries,"sg":_emscripten_glGenQueriesEXT,"af":_emscripten_glGenRenderbuffers,"Nb":_emscripten_glGenSamplers,"$e":_emscripten_glGenTextures,"zb":_emscripten_glGenTransformFeedbacks,"Jc":_emscripten_glGenVertexArrays,"fg":_emscripten_glGenVertexArraysOES,"cf":_emscripten_glGenerateMipmap,"_e":_emscripten_glGetActiveAttrib,"Ze":_emscripten_glGetActiveUniform,"_b":_emscripten_glGetActiveUniformBlockName,"$b":_emscripten_glGetActiveUniformBlockiv,"bc":_emscripten_glGetActiveUniformsiv,"Ye":_emscripten_glGetAttachedShaders,"Xe":_emscripten_glGetAttribLocation,"We":_emscripten_glGetBooleanv,"Ob":_emscripten_glGetBufferParameteri64v,"Ve":_emscripten_glGetBufferParameteriv,"Yc":_emscripten_glGetBufferPointerv,"Ue":_emscripten_glGetError,"Te":_emscripten_glGetFloatv,"rc":_emscripten_glGetFragDataLocation,"Se":_emscripten_glGetFramebufferAttachmentParameteriv,"Pb":_emscripten_glGetInteger64i_v,"Rb":_emscripten_glGetInteger64v,"Hc":_emscripten_glGetIntegeri_v,"Re":_emscripten_glGetIntegerv,"ob":_emscripten_glGetInternalformativ,"vb":_emscripten_glGetProgramBinary,"Pe":_emscripten_glGetProgramInfoLog,"Qe":_emscripten_glGetProgramiv,"jg":_emscripten_glGetQueryObjecti64vEXT,"lg":_emscripten_glGetQueryObjectivEXT,"ig":_emscripten_glGetQueryObjectui64vEXT,"_c":_emscripten_glGetQueryObjectuiv,"kg":_emscripten_glGetQueryObjectuivEXT,"ad":_emscripten_glGetQueryiv,"mg":_emscripten_glGetQueryivEXT,"Oe":_emscripten_glGetRenderbufferParameteriv,"Db":_emscripten_glGetSamplerParameterfv,"Eb":_emscripten_glGetSamplerParameteriv,"Me":_emscripten_glGetShaderInfoLog,"Le":_emscripten_glGetShaderPrecisionFormat,"Ke":_emscripten_glGetShaderSource,"Ne":_emscripten_glGetShaderiv,"Je":_emscripten_glGetString,"ec":_emscripten_glGetStringi,"Qb":_emscripten_glGetSynciv,"Ie":_emscripten_glGetTexParameterfv,"He":_emscripten_glGetTexParameteriv,"Ac":_emscripten_glGetTransformFeedbackVarying,"ac":_emscripten_glGetUniformBlockIndex,"cc":_emscripten_glGetUniformIndices,"Ee":_emscripten_glGetUniformLocation,"Ge":_emscripten_glGetUniformfv,"Fe":_emscripten_glGetUniformiv,"sc":_emscripten_glGetUniformuiv,"yc":_emscripten_glGetVertexAttribIiv,"xc":_emscripten_glGetVertexAttribIuiv,"Be":_emscripten_glGetVertexAttribPointerv,"De":_emscripten_glGetVertexAttribfv,"Ce":_emscripten_glGetVertexAttribiv,"Ae":_emscripten_glHint,"sb":_emscripten_glInvalidateFramebuffer,"rb":_emscripten_glInvalidateSubFramebuffer,"ze":_emscripten_glIsBuffer,"ye":_emscripten_glIsEnabled,"xe":_emscripten_glIsFramebuffer,"we":_emscripten_glIsProgram,"dd":_emscripten_glIsQuery,"qg":_emscripten_glIsQueryEXT,"ve":_emscripten_glIsRenderbuffer,"Lb":_emscripten_glIsSampler,"ue":_emscripten_glIsShader,"Vb":_emscripten_glIsSync,"te":_emscripten_glIsTexture,"yb":_emscripten_glIsTransformFeedback,"Ic":_emscripten_glIsVertexArray,"eg":_emscripten_glIsVertexArrayOES,"se":_emscripten_glLineWidth,"re":_emscripten_glLinkProgram,"Nc":_emscripten_glMapBufferRange,"xb":_emscripten_glPauseTransformFeedback,"qe":_emscripten_glPixelStorei,"pe":_emscripten_glPolygonOffset,"ub":_emscripten_glProgramBinary,"tb":_emscripten_glProgramParameteri,"ng":_emscripten_glQueryCounterEXT,"nd":_emscripten_glReadBuffer,"oe":_emscripten_glReadPixels,"ne":_emscripten_glReleaseShaderCompiler,"me":_emscripten_glRenderbufferStorage,"Pc":_emscripten_glRenderbufferStorageMultisample,"wb":_emscripten_glResumeTransformFeedback,"le":_emscripten_glSampleCoverage,"Gb":_emscripten_glSamplerParameterf,"Fb":_emscripten_glSamplerParameterfv,"Ib":_emscripten_glSamplerParameteri,"Hb":_emscripten_glSamplerParameteriv,"ke":_emscripten_glScissor,"je":_emscripten_glShaderBinary,"ie":_emscripten_glShaderSource,"he":_emscripten_glStencilFunc,"ge":_emscripten_glStencilFuncSeparate,"fe":_emscripten_glStencilMask,"ee":_emscripten_glStencilMaskSeparate,"de":_emscripten_glStencilOp,"ce":_emscripten_glStencilOpSeparate,"be":_emscripten_glTexImage2D,"ld":_emscripten_glTexImage3D,"ae":_emscripten_glTexParameterf,"$d":_emscripten_glTexParameterfv,"_d":_emscripten_glTexParameteri,"Zd":_emscripten_glTexParameteriv,"qb":_emscripten_glTexStorage2D,"pb":_emscripten_glTexStorage3D,"Yd":_emscripten_glTexSubImage2D,"jd":_emscripten_glTexSubImage3D,"Bc":_emscripten_glTransformFeedbackVaryings,"Xd":_emscripten_glUniform1f,"Wd":_emscripten_glUniform1fv,"Vd":_emscripten_glUniform1i,"Ud":_emscripten_glUniform1iv,"qc":_emscripten_glUniform1ui,"mc":_emscripten_glUniform1uiv,"Td":_emscripten_glUniform2f,"Sd":_emscripten_glUniform2fv,"Rd":_emscripten_glUniform2i,"Qd":_emscripten_glUniform2iv,"pc":_emscripten_glUniform2ui,"lc":_emscripten_glUniform2uiv,"Pd":_emscripten_glUniform3f,"Od":_emscripten_glUniform3fv,"Nd":_emscripten_glUniform3i,"Md":_emscripten_glUniform3iv,"oc":_emscripten_glUniform3ui,"kc":_emscripten_glUniform3uiv,"Ld":_emscripten_glUniform4f,"Kd":_emscripten_glUniform4fv,"Jd":_emscripten_glUniform4i,"Id":_emscripten_glUniform4iv,"nc":_emscripten_glUniform4ui,"jc":_emscripten_glUniform4uiv,"Zb":_emscripten_glUniformBlockBinding,"Hd":_emscripten_glUniformMatrix2fv,"Wc":_emscripten_glUniformMatrix2x3fv,"Uc":_emscripten_glUniformMatrix2x4fv,"Gd":_emscripten_glUniformMatrix3fv,"Vc":_emscripten_glUniformMatrix3x2fv,"Sc":_emscripten_glUniformMatrix3x4fv,"Ed":_emscripten_glUniformMatrix4fv,"Tc":_emscripten_glUniformMatrix4x2fv,"Rc":_emscripten_glUniformMatrix4x3fv,"Zc":_emscripten_glUnmapBuffer,"Dd":_emscripten_glUseProgram,"Cd":_emscripten_glValidateProgram,"Bd":_emscripten_glVertexAttrib1f,"Ad":_emscripten_glVertexAttrib1fv,"zd":_emscripten_glVertexAttrib2f,"yd":_emscripten_glVertexAttrib2fv,"xd":_emscripten_glVertexAttrib3f,"wd":_emscripten_glVertexAttrib3fv,"vd":_emscripten_glVertexAttrib4f,"ud":_emscripten_glVertexAttrib4fv,"Cb":_emscripten_glVertexAttribDivisor,"ag":_emscripten_glVertexAttribDivisorANGLE,"mb":_emscripten_glVertexAttribDivisorARB,"rd":_emscripten_glVertexAttribDivisorEXT,"nb":_emscripten_glVertexAttribDivisorNV,"wc":_emscripten_glVertexAttribI4i,"uc":_emscripten_glVertexAttribI4iv,"vc":_emscripten_glVertexAttribI4ui,"tc":_emscripten_glVertexAttribI4uiv,"zc":_emscripten_glVertexAttribIPointer,"td":_emscripten_glVertexAttribPointer,"sd":_emscripten_glViewport,"Sb":_emscripten_glWaitSync,"tg":_emscripten_memcpy_big,"ug":_emscripten_resize_heap,"Ha":_fd_close,"fb":_fd_seek,"Ga":_fd_write,"wg":_getentropy,"h":_glActiveTexture,"T":_glAttachShader,"Qg":_glBeginQuery,"B":_glBindBuffer,"qa":_glBindBufferRange,"l":_glBindFramebuffer,"H":_glBindRenderbuffer,"U":_glBindSampler,"k":_glBindTexture,"N":_glBindVertexArray,"Mg":_glBlendEquationSeparate,"Lg":_glBlendFuncSeparate,"oa":_glBlitFramebuffer,"M":_glBufferData,"Y":_glBufferSubData,"Fg":_glClearBufferfi,"O":_glClearBufferfv,"Eg":_glClearBufferiv,"gb":_glClientWaitSync,"ka":_glColorMask,"ba":_glCompileShader,"Ma":_glCompressedTexSubImage2D,"La":_glCompressedTexSubImage3D,"za":_glCreateProgram,"da":_glCreateShader,"ja":_glCullFace,"Ba":_glDeleteBuffers,"S":_glDeleteFramebuffers,"$":_glDeleteProgram,"_a":_glDeleteQueries,"E":_glDeleteRenderbuffers,"ma":_glDeleteSamplers,"L":_glDeleteShader,"F":_glDeleteSync,"J":_glDeleteTextures,"Jb":_glDeleteVertexArrays,"Kg":_glDepthFunc,"Jg":_glDepthMask,"Ya":_glDepthRangef,"_":_glDetachShader,"j":_glDisable,"ra":_glDisableVertexAttribArray,"Qa":_glDrawArrays,"Sa":_glDrawBuffers,"Ta":_glDrawRangeElements,"z":_glEnable,"sa":_glEnableVertexAttribArray,"Pg":_glEndQuery,"Z":_glFenceSync,"Ca":_glFinish,"Da":_glFlush,"P":_glFramebufferRenderbuffer,"Q":_glFramebufferTexture2D,"Pa":_glFramebufferTextureLayer,"Ng":_glFrontFace,"W":_glGenBuffers,"R":_glGenFramebuffers,"$a":_glGenQueries,"ga":_glGenRenderbuffers,"na":_glGenSamplers,"I":_glGenTextures,"eb":_glGenVertexArrays,"Za":_glGenerateMipmap,"Ra":_glGetError,"kd":_glGetFloatv,"n":_glGetIntegerv,"bb":_glGetProgramInfoLog,"xa":_glGetProgramiv,"Og":_glGetQueryObjectuiv,"ab":_glGetShaderInfoLog,"aa":_glGetShaderiv,"V":_glGetString,"Fd":_glGetStringi,"db":_glGetUniformBlockIndex,"wa":_glGetUniformLocation,"Rg":_glGetVertexAttribiv,"$c":_glHint,"ua":_glInvalidateFramebuffer,"la":_glIsEnabled,"ya":_glLinkProgram,"Dg":_glMapBufferRange,"Gc":_glPixelStorei,"Ua":_glPolygonOffset,"pa":_glReadPixels,"ea":_glRenderbufferStorage,"fa":_glRenderbufferStorageMultisample,"t":_glSamplerParameteri,"Gg":_glScissor,"ca":_glShaderSource,"w":_glTexParameteri,"Ig":_glTexStorage2D,"Hg":_glTexStorage3D,"Oa":_glTexSubImage2D,"Na":_glTexSubImage3D,"va":_glUniform1i,"cb":_glUniformBlockBinding,"Cg":_glUnmapBuffer,"K":_glUseProgram,"Va":_glVertexAttrib4f,"Wa":_glVertexAttribI4ui,"Xa":_glVertexAttribIPointer,"ta":_glVertexAttribPointer,"X":_glViewport,"hb":_glWaitSync,"Ea":_pthread_create,"yg":_pthread_join,"Aa":_setTempRet0,"xg":_sysconf};var asm=createWasm();var ___wasm_call_ctors=Module["___wasm_call_ctors"]=function(){return(___wasm_call_ctors=Module["___wasm_call_ctors"]=Module["asm"]["Ug"]).apply(null,arguments)};var _free=Module["_free"]=function(){return(_free=Module["_free"]=Module["asm"]["Vg"]).apply(null,arguments)};var _malloc=Module["_malloc"]=function(){return(_malloc=Module["_malloc"]=Module["asm"]["Wg"]).apply(null,arguments)};var ___getTypeName=Module["___getTypeName"]=function(){return(___getTypeName=Module["___getTypeName"]=Module["asm"]["Xg"]).apply(null,arguments)};var ___embind_register_native_and_builtin_types=Module["___embind_register_native_and_builtin_types"]=function(){return(___embind_register_native_and_builtin_types=Module["___embind_register_native_and_builtin_types"]=Module["asm"]["Yg"]).apply(null,arguments)};var ___errno_location=Module["___errno_location"]=function(){return(___errno_location=Module["___errno_location"]=Module["asm"]["Zg"]).apply(null,arguments)};var dynCall_jii=Module["dynCall_jii"]=function(){return(dynCall_jii=Module["dynCall_jii"]=Module["asm"]["_g"]).apply(null,arguments)};var dynCall_iiij=Module["dynCall_iiij"]=function(){return(dynCall_iiij=Module["dynCall_iiij"]=Module["asm"]["$g"]).apply(null,arguments)};var dynCall_vij=Module["dynCall_vij"]=function(){return(dynCall_vij=Module["dynCall_vij"]=Module["asm"]["ah"]).apply(null,arguments)};var dynCall_jiji=Module["dynCall_jiji"]=function(){return(dynCall_jiji=Module["dynCall_jiji"]=Module["asm"]["bh"]).apply(null,arguments)};var calledRun;function ExitStatus(status){this.name="ExitStatus";this.message="Program terminated with exit("+status+")";this.status=status}dependenciesFulfilled=function runCaller(){if(!calledRun)run();if(!calledRun)dependenciesFulfilled=runCaller};function run(args){args=args||arguments_;if(runDependencies>0){return}preRun();if(runDependencies>0)return;function doRun(){if(calledRun)return;calledRun=true;Module["calledRun"]=true;if(ABORT)return;initRuntime();preMain();readyPromiseResolve(Module);if(Module["onRuntimeInitialized"])Module["onRuntimeInitialized"]();postRun()}if(Module["setStatus"]){Module["setStatus"]("Running...");setTimeout(function(){setTimeout(function(){Module["setStatus"]("")},1);doRun()},1)}else{doRun()}}Module["run"]=run;if(Module["preInit"]){if(typeof Module["preInit"]=="function")Module["preInit"]=[Module["preInit"]];while(Module["preInit"].length>0){Module["preInit"].pop()()}}noExitRuntime=true;run();


  return Filament.ready
}
);
})();
if (typeof exports === 'object' && typeof module === 'object')
  module.exports = Filament;
else if (typeof define === 'function' && define['amd'])
  define([], function() { return Filament; });
else if (typeof exports === 'object')
  exports["Filament"] = Filament;
/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/// init ::function:: Downloads assets, loads the Filament module, and invokes a callback when done.
///
/// All JavaScript clients must call the init function, passing in a list of asset URL's and a
/// callback. This callback gets invoked only after all assets have been downloaded and the Filament
/// WebAssembly module has been loaded. Clients should only pass asset URL's that absolutely must
/// be ready at initialization time.
///
/// When the callback is called, each downloaded asset is available in the `Filament.assets` global
/// object, which contains a mapping from URL's to Uint8Array objects.
///
/// assets ::argument:: Array of strings containing URL's of required assets.
/// onready ::argument:: callback that gets invoked after all assets have been downloaded and the \
/// Filament WebAssembly module has been loaded.
Filament.init = (assets, onready) => {
    onready = onready || (() => {});
    Filament.assets = {};

    // Usage of glmatrix is optional. If it exists, then go ahead and augment it with some
    // useful math functions.
    if (typeof glMatrix !== 'undefined') {
        Filament.loadMathExtensions();
    }

    // One task for compiling & loading the wasm file, plus one task for each asset.
    let remainingTasks = 1 + assets.length;
    const taskFinished = () => {
        if (--remainingTasks == 0) {
            onready();
        }
    };

    // Issue a fetch for each asset.
    Filament.fetch(assets, null, taskFinished);

    // Emscripten creates a global function called "Filament" that returns a promise that
    // resolves to a module. Here we replace the function with the module. Note that our
    // TypeScript bindings assume that Filament is a namespace, not a function.
    Filament().then(module => {
        Filament = Object.assign(module, Filament);

        // At this point, emscripten has finished compiling and instancing the WebAssembly module.
        // The JS classes that correspond to core Filament classes (e.g., Engine) are not guaranteed
        // to exist until now.

        Filament.loadClassExtensions();
        taskFinished();
    });
};

Filament.clearAssetCache = () => {
    for (const key in Filament.assets) delete Filament.assets[key];
};

/// fetch ::function:: Downloads assets and invokes a callback when done.
///
/// This utility consumes an array of URI strings and invokes callbacks after each asset is
/// downloaded. Additionally, each downloaded asset becomes available in the `Filament.assets`
/// global object, which is a mapping from URI strings to `Uint8Array`. If desired, clients can
/// pre-populate entries in `Filament.assets` to circumvent HTTP requests (this should be done after
/// calling `Filament.init`).
///
/// This function is used internally by `Filament.init` and `gltfio$FilamentAsset.loadResources`.
///
/// assets ::argument:: Array of strings containing URL's of required assets.
/// onDone ::argument:: callback that gets invoked after all assets have been downloaded.
/// onFetched ::argument:: optional callback that's invoked after each asset is downloaded.
Filament.fetch = (assets, onDone, onFetched) => {
    let remainingAssets = assets.length;
    assets.forEach(name => {

        // Check if a buffer already exists in case the client wishes
        // to provide its own data rather than using a HTTP request.
        if (Filament.assets[name]) {
            if (onFetched) {
                onFetched(name);
            }
            if (--remainingAssets === 0 && onDone) {
                onDone();
            }
        } else {
            fetch(name).then(response => {
                if (!response.ok) {
                    throw new Error(name);
                }
                return response.arrayBuffer();
            }).then(arrayBuffer => {
                Filament.assets[name] = new Uint8Array(arrayBuffer);
                if (onFetched) {
                    onFetched(name);
                }
                if (--remainingAssets === 0 && onDone) {
                    onDone();
                }
            });
        }
    });
};

/*
* Copyright (C) 2018 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

// Private utility that converts an asset string or Uint8Array into a low-level buffer descriptor.
// Note that the low-level buffer descriptor must be manually deleted.
function getBufferDescriptor(buffer) {
    if ('string' == typeof buffer || buffer instanceof String) {
        buffer = Filament.assets[buffer];
    }
    if (buffer.buffer instanceof ArrayBuffer) {
        buffer = Filament.Buffer(buffer);
    }
    return buffer;
}

Filament.vectorToArray = function(vector) {
    const result = [];
    for (let i = 0; i < vector.size(); i++) {
        result.push(vector.get(i));
    }
    return result;
};

Filament.shadowOptions = function(overrides) {
    const options = {
        mapSize: 1024,
        shadowCascades: 1,
        constantBias: 0.001,
        normalBias: 1.0,
        shadowFar: 0.0,
        shadowNearHint: 1.0,
        shadowFarHint: 100.0,
        stable: false,
        polygonOffsetConstant: 0.5,
        polygonOffsetSlope: 2.0,
        screenSpaceContactShadows: false,
        stepCount: 8,
        maxShadowDistance: 0.3
    };
    return Object.assign(options, overrides);
};

Filament.loadClassExtensions = function() {

    /// Engine ::core class::

    /// create ::static method:: Creates an Engine instance for the given canvas.
    /// canvas ::argument:: the canvas DOM element
    /// options ::argument:: optional WebGL 2.0 context configuration
    /// ::retval:: an instance of [Engine]
    Filament.Engine.create = function(canvas, options) {
        const defaults = {
            majorVersion: 2,
            minorVersion: 0,
            antialias: false,
            depth: true,
            alpha: false
        };
        options = Object.assign(defaults, options);

        // Create the WebGL 2.0 context.
        const ctx = canvas.getContext("webgl2", options);
        Filament.glOptions = options;
        Filament.glContext = ctx;

        // Enable all desired extensions by calling getExtension on each one.
        ctx.getExtension('WEBGL_compressed_texture_s3tc');
        ctx.getExtension('WEBGL_compressed_texture_astc');
        ctx.getExtension('WEBGL_compressed_texture_etc');

        // Register the GL context with emscripten and create the Engine.
        return Filament.Engine._create();
    };

    /// createMaterial ::method::
    /// package ::argument:: asset string, or Uint8Array, or [Buffer] with filamat contents
    /// ::retval:: an instance of [createMaterial]
    Filament.Engine.prototype.createMaterial = function(buffer) {
        buffer = getBufferDescriptor(buffer);
        const result = this._createMaterial(buffer);
        buffer.delete();
        return result;
    };

    /// createTextureFromKtx ::method:: Utility function that creates a [Texture] from a KTX file.
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer] with KTX file contents
    /// options ::argument:: Options dictionary.
    /// ::retval:: [Texture]
    Filament.Engine.prototype.createTextureFromKtx = function(buffer, options) {
        buffer = getBufferDescriptor(buffer);
        const result = Filament._createTextureFromKtx(buffer, this, options);
        buffer.delete();
        return result;
    };

    /// createIblFromKtx ::method:: Utility that creates an [IndirectLight] from a KTX file.
    /// NOTE: To prevent a leak, please be sure to destroy the associated reflections texture.
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer] with KTX file contents
    /// options ::argument:: Options dictionary.
    /// ::retval:: [IndirectLight]
    Filament.Engine.prototype.createIblFromKtx = function(buffer, options) {
        buffer = getBufferDescriptor(buffer);
        const result = Filament._createIblFromKtx(buffer, this, options);
        buffer.delete();
        return result;
    };

    /// createSkyFromKtx ::method:: Utility function that creates a [Skybox] from a KTX file.
    /// NOTE: To prevent a leak, please be sure to destroy the associated texture.
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer] with KTX file contents
    /// options ::argument:: Options dictionary.
    /// ::retval:: [Skybox]
    Filament.Engine.prototype.createSkyFromKtx = function(buffer, options) {
        const skytex = this.createTextureFromKtx(buffer, options);
        return Filament.Skybox.Builder().environment(skytex).build(this);
    };

    /// createTextureFromPng ::method:: Creates a 2D [Texture] from the raw contents of a PNG file.
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer] with PNG file contents
    /// options ::argument:: object with optional `srgb`, `noalpha`, and `nomips` keys.
    /// ::retval:: [Texture]
    Filament.Engine.prototype.createTextureFromPng = function(buffer, options) {
        buffer = getBufferDescriptor(buffer);
        const result = Filament._createTextureFromImageFile(buffer, this, options);
        buffer.delete();
        return result;
    };

    /// createTextureFromJpeg ::method:: Creates a 2D [Texture] from the contents of a JPEG file.
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer] with JPEG file contents
    /// options ::argument:: JavaScript object with optional `srgb` and `nomips` keys.
    /// ::retval:: [Texture]
    Filament.Engine.prototype.createTextureFromJpeg = function(buffer, options) {
        buffer = getBufferDescriptor(buffer);
        const result = Filament._createTextureFromImageFile(buffer, this, options);
        buffer.delete();
        return result;
    };

    /// loadFilamesh ::method:: Consumes the contents of a filamesh file and creates a renderable.
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer] with filamesh contents
    /// definstance ::argument:: Optional default [MaterialInstance]
    /// matinstances ::argument:: Optional in-out object that gets populated with a \
    /// name-to-[MaterialInstance] mapping. Clients can also optionally provide individual \
    /// material instances using this argument.
    /// ::retval:: JavaScript object with keys `renderable`, `vertexBuffer`, and `indexBuffer`. \
    /// These are of type [Entity], [VertexBuffer], and [IndexBuffer].
    Filament.Engine.prototype.loadFilamesh = function(buffer, definstance, matinstances) {
        buffer = getBufferDescriptor(buffer);
        const result = Filament._loadFilamesh(this, buffer, definstance, matinstances);
        buffer.delete();
        return result;
    };

    /// createAssetLoader ::method::
    /// ::retval:: an instance of [AssetLoader]
    /// Clients should create only one asset loader for the lifetime of their app, this prevents
    /// memory leaks and duplication of Material objects.
    Filament.Engine.prototype.createAssetLoader = function() {
        const materials = new Filament.gltfio$UbershaderLoader(this);
        return new Filament.gltfio$AssetLoader(this, materials);
    };

    /// addEntities ::method::
    /// entities ::argument:: array of entities
    /// This method is equivalent to calling `addEntity` on each item in the array.
    Filament.Scene.prototype.addEntities = function(entities) {
        const vector = new Filament.EntityVector();
        for (const entity of entities) {
            vector.push_back(entity);
        }
        this._addEntities(vector);
    };

    /// removeEntities ::method::
    /// entities ::argument:: array of entities
    /// This method is equivalent to calling `remove` on each item in the array.
    Filament.Scene.prototype.removeEntities = function(entities) {
        const vector = new Filament.EntityVector();
        for (const entity of entities) {
            vector.push_back(entity);
        }
        this._removeEntities(vector);
    };

    /// setShadowOptions ::method::
    /// instance ::argument:: Instance of a light component obtained from `getInstance`.
    /// overrides ::argument:: Dictionary with one or more of the following properties: \
    /// mapSize, shadowCascades, constantBias, normalBias, shadowFar, shadowNearHint, \
    /// shadowFarHint, stable, polygonOffsetConstant, polygonOffsetSlope, \
    // screenSpaceContactShadows, stepCount, maxShadowDistance.
    Filament.LightManager.prototype.setShadowOptions = function(instance, overrides) {
        this._setShadowOptions(instance, Filament.shadowOptions(overrides));
    };

    /// setClearOptions ::method::
    /// overrides ::argument:: Dictionary with one or more of the following properties: \
    /// clearColor, clear, discard.
    Filament.Renderer.prototype.setClearOptions = function(overrides) {
        const options = {
            clearColor: [0, 0, 0, 0],
            clear: false,
            discard: true
        };
        Object.assign(options, overrides);
        this._setClearOptions(options);
    };

    /// setAmbientOcclusionOptions ::method::
    /// overrides ::argument:: Dictionary with one or more of the following properties: \
    /// radius, power, bias, resolution, intensity, quality.
    Filament.View.prototype.setAmbientOcclusionOptions = function(overrides) {
        const options = {
            radius: 0.3,
            power: 1.0,
            bias: 0.0005,
            resolution: 0.5,
            intensity: 1.0,
            quality: Filament.View$QualityLevel.LOW
        };
        Object.assign(options, overrides);
        this._setAmbientOcclusionOptions(options);
    };

    /// setDepthOfFieldOptions ::method::
    /// overrides ::argument:: Dictionary with one or more of the following properties: \
    /// focusDistance, cocScale, maxApertureDiameter, enabled.
    Filament.View.prototype.setDepthOfFieldOptions = function(overrides) {
        const options = {
            focusDistance: 10.0,
            cocScale: 1.0,
            maxApertureDiameter: 0.01,
            enabled: false
        };
        Object.assign(options, overrides);
        this._setDepthOfFieldOptions(options);
    };

    /// setBloomOptions ::method::
    /// overrides ::argument:: Dictionary with one or more of the following properties: \
    /// enabled, strength, resolution, anomorphism, levels, blendMode, threshold, highlight.
    /// NOTE: dirt texture is not yet supported in the JavaScript API.
    Filament.View.prototype.setBloomOptions = function(overrides) {
        const options = {
            dirtStrength: 0.2,
            highlight: 1000.0,
            strength: 0.10,
            resolution: 360,
            anamorphism: 1.0,
            levels: 6,
            blendMode: Filament.View$BloomOptions$BlendMode.ADD,
            threshold: true,
            enabled: false,
            dirt: null
        };
        Object.assign(options, overrides);
        this._setBloomOptions(options);
    };

    /// setFogOptions ::method::
    /// overrides ::argument:: Dictionary with one or more of the following properties: \
    /// distance, maximumOpacity, height, heightFalloff, color, density, inScatteringStart,
    /// inScatteringSize, fogColorFromIbl, enabled.
    Filament.View.prototype.setFogOptions = function(overrides) {
        const options = {
            distance:  0.0,
            maximumOpacity:  1.0,
            height:  0.0,
            heightFalloff:  1.0,
            color: .5,
            density:  0.1,
            inScatteringStart:  0.0,
            inScatteringSize:  -1.0,
            fogColorFromIbl:  false,
            enabled:  false
        };
        Object.assign(options, overrides);
        this._setFogOptions(options);
    };

    /// setVignetteOptions ::method::
    /// overrides ::argument:: Dictionary with one or more of the following properties: \
    /// midPoint, roundness, feather, color, enabled.
    Filament.View.prototype.setVignetteOptions = function(overrides) {
        const options = {
            midPoint: 0.5,
            roundness: 0.5,
            feather: 0.5,
            color: [0, 0, 0, 1],
            enabled: false
        };
        Object.assign(options, overrides);
        this._setVignetteOptions(options);
    };

    /// VertexBuffer ::core class::

    /// setBufferAt ::method::
    /// engine ::argument:: [Engine]
    /// bufferIndex ::argument:: non-negative integer
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer]
    /// byteOffset ::argument:: non-negative integer
    Filament.VertexBuffer.prototype.setBufferAt = function(engine, bufferIndex, buffer, byteOffset = 0) {
        buffer = getBufferDescriptor(buffer);
        this._setBufferAt(engine, bufferIndex, buffer, byteOffset);
        buffer.delete();
    };

    /// IndexBuffer ::core class::

    /// setBuffer ::method::
    /// engine ::argument:: [Engine]
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer]
    /// byteOffset ::argument:: non-negative integer
    Filament.IndexBuffer.prototype.setBuffer = function(engine, buffer, byteOffset = 0) {
        buffer = getBufferDescriptor(buffer);
        this._setBuffer(engine, buffer, byteOffset);
        buffer.delete();
    };

    Filament.LightManager$Builder.prototype.shadowOptions = function(overrides) {
        return this._shadowOptions(Filament.shadowOptions(overrides));
    };

    Filament.RenderableManager$Builder.prototype.build =
    Filament.LightManager$Builder.prototype.build =
        function(engine, entity) {
            const result = this._build(engine, entity);
            this.delete();
            return result;
        };

    Filament.ColorGrading$Builder.prototype.build =
    Filament.RenderTarget$Builder.prototype.build =
    Filament.VertexBuffer$Builder.prototype.build =
    Filament.IndexBuffer$Builder.prototype.build =
    Filament.Texture$Builder.prototype.build =
    Filament.IndirectLight$Builder.prototype.build =
    Filament.Skybox$Builder.prototype.build =
        function(engine) {
            const result = this._build(engine);
            this.delete();
            return result;
        };

    Filament.KtxBundle.prototype.getBlob = function(index) {
        const blob = this._getBlob(index);
        const result = blob.getBytes();
        blob.delete();
        return result;
    }

    Filament.KtxBundle.prototype.getCubeBlob = function(miplevel) {
        const blob = this._getCubeBlob(miplevel);
        const result = blob.getBytes();
        blob.delete();
        return result;
    }

    Filament.Texture.prototype.setImage = function(engine, level, pbd) {
        this._setImage(engine, level, pbd);
        pbd.delete();
    }

    Filament.Texture.prototype.setImageCube = function(engine, level, pbd) {
        this._setImageCube(engine, level, pbd);
        pbd.delete();
    }

    Filament.SurfaceOrientation$Builder.prototype.normals = function(buffer, stride = 0) {
        buffer = new Uint8Array(buffer.buffer, buffer.byteOffset, buffer.byteLength);
        this.norPointer = Filament._malloc(buffer.byteLength);
        Filament.HEAPU8.set(buffer, this.norPointer);
        this._normals(this.norPointer, stride);
    };

    Filament.SurfaceOrientation$Builder.prototype.uvs = function(buffer, stride = 0) {
        buffer = new Uint8Array(buffer.buffer, buffer.byteOffset, buffer.byteLength);
        this.uvsPointer = Filament._malloc(buffer.byteLength);
        Filament.HEAPU8.set(buffer, this.uvsPointer);
        this._uvs(this.uvsPointer, stride);
    };

    Filament.SurfaceOrientation$Builder.prototype.positions = function(buffer, stride = 0) {
        buffer = new Uint8Array(buffer.buffer, buffer.byteOffset, buffer.byteLength);
        this.posPointer = Filament._malloc(buffer.byteLength);
        Filament.HEAPU8.set(buffer, this.posPointer);
        this._positions(this.posPointer, stride);
    };

    Filament.SurfaceOrientation$Builder.prototype.triangles16 = function(buffer, stride = 0) {
        buffer = new Uint8Array(buffer.buffer, buffer.byteOffset, buffer.byteLength);
        this.t16Pointer = Filament._malloc(buffer.byteLength);
        Filament.HEAPU8.set(buffer, this.t16Pointer);
        this._triangles16(this.t16Pointer, stride);
    };

    Filament.SurfaceOrientation$Builder.prototype.triangles32 = function(buffer, stride = 0) {
        buffer = new Uint8Array(buffer.buffer, buffer.byteOffset, buffer.byteLength);
        this.t32Pointer = Filament._malloc(buffer.byteLength);
        Filament.HEAPU8.set(buffer, this.t32Pointer);
        this._triangles32(this.t32Pointer, stride);
    };

    Filament.SurfaceOrientation$Builder.prototype.build = function() {
        const result = this._build();
        this.delete();
        if ('norPointer' in this) Filament._free(this.norPointer);
        if ('uvsPointer' in this) Filament._free(this.uvsPointer);
        if ('posPointer' in this) Filament._free(this.posPointer);
        if ('t16Pointer' in this) Filament._free(this.t16Pointer);
        if ('t32Pointer' in this) Filament._free(this.t32Pointer);
        return result;
    };

    Filament.SurfaceOrientation.prototype.getQuats = function(nverts) {
        const attribType = Filament.VertexBuffer$AttributeType.SHORT4;
        const quatsBufferSize = 8 * nverts;
        const quatsBuffer = Filament._malloc(quatsBufferSize);
        this._getQuats(quatsBuffer, nverts, attribType);
        const arrayBuffer = Filament.HEAPU8.subarray(quatsBuffer, quatsBuffer + quatsBufferSize).slice().buffer;
        Filament._free(quatsBuffer);
        return new Int16Array(arrayBuffer);
    };

    Filament.gltfio$AssetLoader.prototype.createAssetFromJson = function(buffer) {
        if ('string' == typeof buffer && buffer.endsWith('.glb')) {
            console.error('Please use createAssetFromBinary for glb files.');
        }
        buffer = getBufferDescriptor(buffer);
        const result = this._createAssetFromJson(buffer);
        buffer.delete();
        return result;
    };

    Filament.gltfio$AssetLoader.prototype.createAssetFromBinary = function(buffer) {
        if ('string' == typeof buffer && buffer.endsWith('.gltf')) {
            console.error('Please use createAssetFromJson for gltf files.');
        }
        buffer = getBufferDescriptor(buffer);
        const result = this._createAssetFromBinary(buffer);
        buffer.delete();
        return result;
    };

    Filament.gltfio$AssetLoader.prototype.createInstancedAsset = function(buffer, instances) {
        buffer = getBufferDescriptor(buffer);
        const asset = this._createInstancedAsset(buffer, instances.length);
        buffer.delete();
        const instancesVector = asset._getAssetInstances();
        for (let i = 0; i < instancesVector.size(); i++) {
            instances[i] = instancesVector.get(i);
        }
        return asset;
    };

    // See the C++ documentation for ResourceLoader and AssetLoader. The JavaScript API differs in
    // that it takes two optional callbacks:
    //
    // - onDone is called after all resources have been downloaded and decoded.
    // - onFetched is called after each resource has finished downloading.
    //
    // Takes an optional base path for resolving the URI strings in the glTF file, which is
    // typically the path to the parent glTF file. The given base path cannot itself be a relative
    // URL, but clients can do the following to resolve a relative URL:
    //    const basePath = '' + new URL(myRelativeUrl, document.location);
    // If the given base path is null, document.location is used as the base.
    //
    // The optional asyncInterval argument allows clients to control how decoding is amortized
    // over time. It represents the number of milliseconds between each texture decoding task.
    //
    // The optional config argument is an object with boolean fields `normalizeSkinningWeights` and
    // `recomputeBoundingBoxes`.
    Filament.gltfio$FilamentAsset.prototype.loadResources = function(onDone, onFetched, basePath,
            asyncInterval, config) {
        const asset = this;
        const engine = this.getEngine();
        const names = this.getResourceUris();
        const interval = asyncInterval || 30;
        const defaults = {
            normalizeSkinningWeights: true,
            recomputeBoundingBoxes: false
        };
        config = Object.assign(defaults, config || {});

        basePath = basePath || document.location;
        onFetched = onFetched || ((name) => {});
        onDone = onDone || (() => {});

        // Construct the set of URI strings to fetch.
        const urlset = new Set();
        const urlToName = {};
        for (let i = 0; i < names.size(); i++) {
            const name = names.get(i);
            if (name) {
                const url = '' + new URL(name, basePath);
                urlToName[url] = name;
                urlset.add(url);
            }
        }

        // Construct a resource loader and start decoding after all textures are fetched.
        const resourceLoader = new Filament.gltfio$ResourceLoader(engine,
                config.normalizeSkinningWeights,
                config.recomputeBoundingBoxes);
        const onComplete = () => {
            resourceLoader.asyncBeginLoad(asset);

            // NOTE: This decodes in the wasm layer instead of using Canvas2D, which allows Filament
            // to have more control (handling of alpha, srgb, etc) and improves parity with native
            // platforms. In the future we may wish to offload this to web workers.

            // Decode a single PNG or JPG every 30 milliseconds, or at the specified interval.
            const timer = setInterval(() => {
                resourceLoader.asyncUpdateLoad();
                const progress = resourceLoader.asyncGetLoadProgress();
                if (progress >= 1) {
                    clearInterval(timer);
                    resourceLoader.delete();
                    onDone();
                }
            }, interval);
        };

        if (urlset.size == 0) {
            onComplete();
            return;
        }

        // Begin downloading all external resources.
        Filament.fetch(Array.from(urlset), onComplete, function(url) {
            const buffer = getBufferDescriptor(url);
            const name = urlToName[url];
            resourceLoader.addResourceData(name, buffer);
            buffer.delete();
            onFetched(name);
        });
    };

    Filament.gltfio$FilamentAsset.prototype.getEntities = function() {
        return Filament.vectorToArray(this._getEntities());
    };

    Filament.gltfio$FilamentAsset.prototype.getEntitiesByName = function(name) {
        return Filament.vectorToArray(this._getEntitiesByName(name));
    };

    Filament.gltfio$FilamentAsset.prototype.getEntitiesByPrefix = function(prefix) {
        return Filament.vectorToArray(this._getEntitiesByPrefix(prefix));
    };

    Filament.gltfio$FilamentAsset.prototype.getLightEntities = function() {
        return Filament.vectorToArray(this._getLightEntities());
    };

    Filament.gltfio$FilamentAsset.prototype.getCameraEntities = function() {
        return Filament.vectorToArray(this._getCameraEntities());
    };
};

/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// ---------------
// Buffer Wrappers
// ---------------

// These wrappers make it easy for JavaScript clients to pass large swaths of data to Filament. They
// copy the contents of the given typed array into the WASM heap, then return a low-level buffer
// descriptor object. If the given array was taken from the WASM heap, then they create a temporary
// copy because the input pointer becomes invalidated after allocating heap memory for the buffer
// descriptor.

/// Buffer ::function:: Constructs a [BufferDescriptor] by copying a typed array into the WASM heap.
/// typedarray ::argument:: Data to consume (e.g. Uint8Array, Uint16Array, Float32Array)
/// ::retval:: [BufferDescriptor]
Filament.Buffer = function(typedarray) {
    console.assert(typedarray.buffer instanceof ArrayBuffer);
    console.assert(typedarray.byteLength > 0);
    if (Filament.HEAPU32.buffer == typedarray.buffer) {
        typedarray = new Uint8Array(typedarray);
    }
    const ta = typedarray;
    const bd = new Filament.driver$BufferDescriptor(ta.byteLength);
    const uint8array = new Uint8Array(ta.buffer, ta.byteOffset, ta.byteLength);
    bd.getBytes().set(uint8array);
    return bd;
};

/// PixelBuffer ::function:: Constructs a [PixelBufferDescriptor] by copying a typed array into \
/// the WASM heap.
/// typedarray ::argument:: Data to consume (e.g. Uint8Array, Uint16Array, Float32Array)
/// format ::argument:: [PixelDataFormat]
/// datatype ::argument:: [PixelDataType]
/// ::retval:: [PixelBufferDescriptor]
Filament.PixelBuffer = function(typedarray, format, datatype) {
    console.assert(typedarray.buffer instanceof ArrayBuffer);
    console.assert(typedarray.byteLength > 0);
    if (Filament.HEAPU32.buffer == typedarray.buffer) {
        typedarray = new Uint8Array(typedarray);
    }
    const ta = typedarray;
    const bd = new Filament.driver$PixelBufferDescriptor(ta.byteLength, format, datatype);
    const uint8array = new Uint8Array(ta.buffer, ta.byteOffset, ta.byteLength);
    bd.getBytes().set(uint8array);
    return bd;
};

/// CompressedPixelBuffer ::function:: Constructs a [PixelBufferDescriptor] for compressed texture
/// data by copying a typed array into the WASM heap.
/// typedarray ::argument:: Data to consume (e.g. Uint8Array, Uint16Array, Float32Array)
/// cdatatype ::argument:: [CompressedPixelDataType]
/// faceSize ::argument:: Number of bytes in each face (cubemaps only)
/// ::retval:: [PixelBufferDescriptor]
Filament.CompressedPixelBuffer = function(typedarray, cdatatype, faceSize) {
    console.assert(typedarray.buffer instanceof ArrayBuffer);
    console.assert(typedarray.byteLength > 0);
    faceSize = faceSize || typedarray.byteLength;
    if (Filament.HEAPU32.buffer == typedarray.buffer) {
        typedarray = new Uint8Array(typedarray);
    }
    const ta = typedarray;
    const bd = new Filament.driver$PixelBufferDescriptor(ta.byteLength, cdatatype, faceSize, true);
    const uint8array = new Uint8Array(ta.buffer, ta.byteOffset, ta.byteLength);
    bd.getBytes().set(uint8array);
    return bd;
};

Filament._loadFilamesh = function(engine, buffer, definstance, matinstances) {
    matinstances = matinstances || {};
    const registry = new Filament.MeshReader$MaterialRegistry();
    for (let key in matinstances) {
        registry.set(key, matinstances[key]);
    }
    if (definstance) {
        registry.set("DefaultMaterial", definstance);
    }
    const mesh = Filament.MeshReader.loadMeshFromBuffer(engine, buffer, registry);
    const keys = registry.keys();
    for (let i = 0; i < keys.size(); i++) {
        const key = keys.get(i);
        const minstance = registry.get(key);
        matinstances[key] = minstance;
    }
    return {
        "renderable": mesh.renderable(),
        "vertexBuffer": mesh.vertexBuffer(),
        "indexBuffer": mesh.indexBuffer(),
    }
}

// ------------------
// Geometry Utilities
// ------------------

/// IcoSphere ::class:: Utility class for constructing spheres (requires glMatrix).
///
/// The constructor takes an integer subdivision level, with 0 being an icosahedron.
///
/// Exposes three arrays as properties:
///
/// - `icosphere.vertices` Float32Array of XYZ coordinates.
/// - `icosphere.tangents` Uint16Array (interpreted as half-floats) encoding the surface orientation
/// as quaternions.
/// - `icosphere.triangles` Uint16Array with triangle indices.
///
Filament.IcoSphere = function(nsubdivs) {
    const X = .525731112119133606;
    const Z = .850650808352039932;
    const N = 0.;
    this.vertices = new Float32Array([
        -X, +N, +Z, +X, +N, +Z, -X, +N, -Z, +X, +N, -Z ,
        +N, +Z, +X, +N, +Z, -X, +N, -Z, +X, +N, -Z, -X ,
        +Z, +X, +N, -Z, +X, +N, +Z, -X, +N, -Z, -X, +N ,
    ]);
    this.triangles = new Uint16Array([
        1,   4, 0,  4,  9,  0, 4,   5, 9, 8, 5,   4 ,  1,  8,  4 ,
        1,  10, 8, 10,  3,  8, 8,   3, 5, 3, 2,   5 ,  3,  7,  2 ,
        3,  10, 7, 10,  6,  7, 6,  11, 7, 6, 0,  11 ,  6,  1,  0 ,
        10,   1, 6, 11,  0,  9, 2,  11, 9, 5, 2,   9 , 11,  2,  7 ,
    ]);

    nsubdivs = nsubdivs || 0;
    while (nsubdivs-- > 0) {
        this.subdivide();
    }

    const nverts = this.vertices.length / 3;

    // This is a unit sphere, so normals = positions.
    const normals = this.vertices;

    // Perform computations.
    const sob = new Filament.SurfaceOrientation$Builder();
    sob.vertexCount(nverts);
    sob.normals(normals, 0)
    const orientation = sob.build();

    // Copy the results out of the helper.
    this.tangents = orientation.getQuats(nverts);

    // Free up the surface orientation helper now that we're done with it.
    orientation.delete();
}

Filament.IcoSphere.prototype.subdivide = function() {
    const srctris = this.triangles;
    const srcverts = this.vertices;
    const nsrctris = srctris.length / 3;
    const ndsttris = nsrctris * 4;
    const nsrcverts = srcverts.length / 3;
    const ndstverts = nsrcverts + nsrctris * 3;
    const dsttris = new Uint16Array(ndsttris * 3);
    const dstverts = new Float32Array(ndstverts * 3);
    dstverts.set(srcverts);
    let srcind = 0, dstind = 0, i3 = nsrcverts * 3, i4 = i3 + 3, i5 = i4 + 3;
    for (let tri = 0; tri < nsrctris; tri++, i3 += 9, i4 += 9, i5 += 9) {
        const i0 = srctris[srcind++] * 3;
        const i1 = srctris[srcind++] * 3;
        const i2 = srctris[srcind++] * 3;
        const v0 = srcverts.subarray(i0, i0 + 3);
        const v1 = srcverts.subarray(i1, i1 + 3);
        const v2 = srcverts.subarray(i2, i2 + 3);
        const v3 = dstverts.subarray(i3, i3 + 3);
        const v4 = dstverts.subarray(i4, i4 + 3);
        const v5 = dstverts.subarray(i5, i5 + 3);
        vec3.normalize(v3, vec3.add(v3, v0, v1));
        vec3.normalize(v4, vec3.add(v4, v1, v2));
        vec3.normalize(v5, vec3.add(v5, v2, v0));
        dsttris[dstind++] = i0 / 3;
        dsttris[dstind++] = i3 / 3;
        dsttris[dstind++] = i5 / 3;
        dsttris[dstind++] = i3 / 3;
        dsttris[dstind++] = i1 / 3;
        dsttris[dstind++] = i4 / 3;
        dsttris[dstind++] = i5 / 3;
        dsttris[dstind++] = i3 / 3;
        dsttris[dstind++] = i4 / 3;
        dsttris[dstind++] = i2 / 3;
        dsttris[dstind++] = i5 / 3;
        dsttris[dstind++] = i4 / 3;
    }
    this.triangles = dsttris;
    this.vertices = dstverts;
}

// ---------------
// Math Extensions
// ---------------

function clamp(v, least, most) {
    return Math.max(Math.min(most, v), least);
}

/// packSnorm16 ::function:: Converts a float in [-1, +1] into a half-float.
/// value ::argument:: float
/// ::retval:: half-float
Filament.packSnorm16 = function(value) {
    return Math.round(clamp(value, -1.0, 1.0) * 32767.0);
}

/// loadMathExtensions ::function:: Extends the [glMatrix](http://glmatrix.net/) math library.
/// Filament does not require its clients to use glMatrix, but if its usage is detected then
/// the [init] function will automatically call `loadMathExtensions`.
/// This defines the following functions:
/// - **vec4.packSnorm16** can be used to create half-floats (see [packSnorm16])
/// - **mat3.fromRotation** now takes an arbitrary axis
Filament.loadMathExtensions = function() {
    vec4.packSnorm16 = function(out, src) {
        out[0] = Filament.packSnorm16(src[0]);
        out[1] = Filament.packSnorm16(src[1]);
        out[2] = Filament.packSnorm16(src[2]);
        out[3] = Filament.packSnorm16(src[3]);
        return out;
    }
    // In gl-matrix, mat3 rotation assumes rotation about the Z axis, so here we add a function
    // to allow an arbitrary axis.
    const fromRotationZ = mat3.fromRotation;
    mat3.fromRotation = function(out, radians, axis) {
        if (axis) {
            return mat3.fromMat4(out, mat4.fromRotation(mat4.create(), radians, axis));
        }
        return fromRotationZ(out, radians);
    };
};

// ---------------
// Texture helpers
// ---------------

Filament._createTextureFromKtx = function(ktxdata, engine, options) {
    options = options || {};
    const ktx = options['ktx'] || new Filament.KtxBundle(ktxdata);
    const srgb = !!options['srgb'];
    return Filament.ktx$createTexture(engine, ktx, srgb);
};

Filament._createIblFromKtx = function(ktxdata, engine, options) {
    options = options || {};
    const iblktx = options['ktx'] = new Filament.KtxBundle(ktxdata);

    const format = iblktx.info().glInternalFormat;
    if (format != this.ctx.R11F_G11F_B10F && format != this.ctx.RGB16F && format != this.ctx.RGB32F) {
        console.warn('IBL texture format is 0x' + format.toString(16) +
            ' which is not an expected floating-point format. Please use cmgen to generate IBL.');
    }

    const ibltex = Filament._createTextureFromKtx(ktxdata, engine, options);
    const shstring = iblktx.getMetadata("sh");
    const ibl = Filament.IndirectLight.Builder()
        .reflections(ibltex)
        .build(engine);
    ibl.shfloats = shstring.split(/\s/, 9 * 3).map(parseFloat);
    return ibl;
};

Filament._createTextureFromImageFile = function(fileContents, engine, options) {
    const Sampler = Filament.Texture$Sampler;
    const TextureFormat = Filament.Texture$InternalFormat;
    const PixelDataFormat = Filament.PixelDataFormat;

    options = options || {};
    const srgb = !!options['srgb'];
    const noalpha = !!options['noalpha'];
    const nomips = !!options['nomips'];

    const decodedImage = Filament.decodeImage(fileContents, noalpha ? 3 : 4);

    let texformat, pbformat, pbtype;
    if (noalpha) {
        texformat = srgb ? TextureFormat.SRGB8 : TextureFormat.RGB8;
        pbformat = PixelDataFormat.RGB;
        pbtype = Filament.PixelDataType.UBYTE;
    } else {
        texformat = srgb ? TextureFormat.SRGB8_A8 : TextureFormat.RGBA8;
        pbformat = PixelDataFormat.RGBA;
        pbtype = Filament.PixelDataType.UBYTE;
    }

    const tex = Filament.Texture.Builder()
        .width(decodedImage.width)
        .height(decodedImage.height)
        .levels(nomips ? 1 : 0xff)
        .sampler(Sampler.SAMPLER_2D)
        .format(texformat)
        .build(engine);

    const pixelbuffer = Filament.PixelBuffer(decodedImage.data.getBytes(), pbformat, pbtype);
    tex.setImage(engine, 0, pixelbuffer);
    if (!nomips) {
        tex.generateMipmaps(engine);
    }
    return tex;
};

/// getSupportedFormats ::function:: Queries WebGL to check which compressed formats are supported.
/// ::retval:: object with boolean values and the following keys: s3tc, astc, etc
Filament.getSupportedFormats = function() {
    if (Filament.supportedFormats) {
        return Filament.supportedFormats;
    }
    const options = { majorVersion: 2, minorVersion: 0 };
    let ctx = document.createElement('canvas').getContext('webgl2', options);
    const result = {
        s3tc: false,
        astc: false,
        etc: false,
    }
    let exts = ctx.getSupportedExtensions(), nexts = exts.length, i;
    for (i = 0; i < nexts; i++) {
        let ext = exts[i];
        if (ext == "WEBGL_compressed_texture_s3tc") {
            result.s3tc = true;
        } else if (ext == "WEBGL_compressed_texture_astc") {
            result.astc = true;
        } else if (ext == "WEBGL_compressed_texture_etc") {
            result.etc = true;
        }
    }
    return Filament.supportedFormats = result;
}

/// getSupportedFormatSuffix ::function:: Generate a file suffix according to the texture format.
/// Consumes a string describing desired formats and produces a file suffix depending on
/// which (if any) of the formats are actually supported by the WebGL implementation. This is
/// useful for compressed textures. For example, some platforms accept ETC and others accept S3TC.
/// desiredFormats ::argument:: space-delimited string of desired formats
/// ::retval:: empty string if there is no intersection of supported and desired formats.
Filament.getSupportedFormatSuffix = function(desiredFormats) {
    desiredFormats = desiredFormats.split(' ');
    let exts = Filament.getSupportedFormats();
    for (let key in exts) {
        if (exts[key] && desiredFormats.includes(key)) {
            return '_' + key;
        }
    }
    return '';
}

