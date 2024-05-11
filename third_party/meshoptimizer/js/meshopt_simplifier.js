// This file is part of meshoptimizer library and is distributed under the terms of MIT License.
// Copyright (C) 2016-2022, by Arseny Kapoulkine (arseny.kapoulkine@gmail.com)
var MeshoptSimplifier = (function() {
	"use strict";

	// Built with clang version 14.0.4
	// Built from meshoptimizer 0.18
	var wasm = "b9H79TebbbecD9Geueu9Geub9Gbb9Gquuuuuuu99uueu9Gvuuuuub9Gluuuue999Giuuue999Gluuuueu9Giuuueuimxdilvorbwwbewlve9Weiiviebeoweuecj;jekr7oo9TW9T9VV95dbH9F9F939H79T9F9J9H229F9Jt9VV7bbz9TW79O9V9Wt9F79P9T9W29P9M95beX9TW79O9V9Wt9F79P9T9W29P9M959t9J9H2Wbla9TW79O9V9Wt9F9V9Wt9P9T9P96W9wWVtW94SWt9J9O9sW9T9H9Wbvl79IV9RboDwebcekdDqq:STxdbk;48YiKuP99Hu8Jjjjjbcj;bb9Rgq8KjjjjbaqcKfcbc;Kbz1jjjb8AcualcdtgkalcFFFFi0Egxcbyd;S1jjbHjjjjbbhmaqcKfaqyd94gPcdtfamBdbaqamBdwaqaPcefBd94axcbyd;S1jjbHjjjjbbhsaqcKfaqyd94gPcdtfasBdbaqasBdxaqaPcefBd94cuadcitadcFFFFe0Ecbyd;S1jjbHjjjjbbhzaqcKfaqyd94gPcdtfazBdbaqazBdzaqaPcefBd94aqcwfaeadalcbz:cjjjbaxcbyd;S1jjbHjjjjbbhHaqcKfaqyd94gPcdtfaHBdbaqaPcefBd94axcbyd;S1jjbHjjjjbbhOaqcKfaqyd94gPcdtfaOBdbaqaPcefBd94alcd4alfhAcehCinaCgPcethCaPaA6mbkcbhXcuaPcdtgAaPcFFFFi0Ecbyd;S1jjbHjjjjbbhCaqcKfaqyd94gQcdtfaCBdbaqaQcefBd94aCcFeaAz1jjjbhLdnalTmbavcd4hKaPcufhQinaiaXaK2cdtfgYydlgPcH4aP7c:F:b:DD2aYydbgPcH4aP7c;D;O:B8J27aYydwgPcH4aP7c:3F;N8N27hAcbhPdndninaLaAaQGgAcdtfg8AydbgCcuSmeaiaCaK2cdtfaYcxz:ljjjbTmdaPcefgPaAfhAaPaQ9nmbxdkka8AaXBdbaXhCkaHaXcdtfaCBdbaXcefgXal9hmbkcbhPaOhCinaCaPBdbaCclfhCalaPcefgP9hmbkcbhPaHhCaOhAindnaPaCydbgQSmbaAaOaQcdtfgQydbBdbaQaPBdbkaCclfhCaAclfhAalaPcefgP9hmbkkcbhAalcbyd;S1jjbHjjjjbbhYaqcKfaqyd94gPcdtfaYBdbaqaPcefBd94axcbyd;S1jjbHjjjjbbhPaqcKfaqyd94gCcdtfaPBdbaqaCcefBd94axcbyd;S1jjbHjjjjbbhCaqcKfaqyd94gQcdtfaCBdbaqaQcefBd94aPcFeakz1jjjbhEaCcFeakz1jjjbh3dnalTmbazcwfh5indnamaAcdtgPfydbg8ETmbazasaPfydbcitfh8Fa3aPfhaaEaPfhXcbhKindndna8FaKcitfydbgLaA9hmbaXaABdbaaaABdbxekdnamaLcdtgkfydbghTmbazasakfydbcitgPfydbaASmeahcufh8Aa5aPfhCcbhPina8AaPSmeaPcefhPaCydbhQaCcwfhCaQaA9hmbkaPah6meka3akfgPaAaLaPydbcuSEBdbaXaLaAaXydbcuSEBdbkaKcefgKa8E9hmbkkaAcefgAal9hmbkaHhCaOhAa3hQaEhKcbhPindndnaPaCydbg8A9hmbdnaPaAydbg8A9hmbaKydbh8AdnaQydbgLcu9hmba8Acu9hmbaYaPfcb86bbxikaYaPfhXdnaPaLSmbaPa8ASmbaXce86bbxikaXcl86bbxdkdnaPaOa8AcdtgLfydb9hmbdnaQydbgXcuSmbaPaXSmbaKydbgkcuSmbaPakSmba3aLfydbg8EcuSmba8Ea8ASmbaEaLfydbgLcuSmbaLa8ASmbdnaHaXcdtfydbaHaLcdtfydb9hmbaHakcdtfydbaHa8Ecdtfydb9hmbaYaPfcd86bbxlkaYaPfcl86bbxikaYaPfcl86bbxdkaYaPfcl86bbxekaYaPfaYa8AfRbb86bbkaCclfhCaAclfhAaQclfhQaKclfhKalaPcefgP9hmbkawceGTmbaYhPalhCindnaPRbbce9hmbaPcl86bbkaPcefhPaCcufgCmbkkcbhKcualcx2alc;v:Q;v:Qe0Ecbyd;S1jjbHjjjjbbhmaqcKfaqyd94gPcdtfamBdbaqaPcefBd94amaialavz:djjjb8Acualc8S2gCalc;D;O;f8U0Ecbyd;S1jjbHjjjjbbhPaqcKfaqyd94gAcdtfaPBdbaqaAcefBd94aPcbaCz1jjjbhzdnadTmbaehCindnamaCclfydbg8Acx2fgPIdbamaCydbgLcx2fgAIdbgg:tg8JamaCcwfydbgXcx2fgQclfIdbaAclfIdbg8K:tg8LNaQIdbag:tg8MaPclfIdba8K:tg8NN:tgyayNa8NaQcwfIdbaAcwfIdbg8P:tgINa8LaPcwfIdba8P:tg8NN:tg8La8LNa8Na8MNaIa8JN:tg8Ja8JNMM:rg8MJbbbb9ETmbaya8M:vhya8Ja8M:vh8Ja8La8M:vh8LkazaHaLcdtfydbc8S2fgPa8La8M:rg8Ma8LNNg8NaPIdbMUdbaPa8Ja8Ma8JNg8RNgIaPIdlMUdlaPaya8MayNg8SNgRaPIdwMUdwaPa8Ra8LNg8RaPIdxMUdxaPa8Sa8LNg8UaPIdzMUdzaPa8Sa8JNg8SaPIdCMUdCaPa8La8Maya8PNa8LagNa8Ka8JNMM:mg8KNggNg8LaPIdKMUdKaPa8JagNg8JaPId3MUd3aPayagNgyaPIdaMUdaaPaga8KNggaPId8KMUd8KaPa8MaPIdyMUdyazaHa8Acdtfydbc8S2fgPa8NaPIdbMUdbaPaIaPIdlMUdlaPaRaPIdwMUdwaPa8RaPIdxMUdxaPa8UaPIdzMUdzaPa8SaPIdCMUdCaPa8LaPIdKMUdKaPa8JaPId3MUd3aPayaPIdaMUdaaPagaPId8KMUd8KaPa8MaPIdyMUdyazaHaXcdtfydbc8S2fgPa8NaPIdbMUdbaPaIaPIdlMUdlaPaRaPIdwMUdwaPa8RaPIdxMUdxaPa8UaPIdzMUdzaPa8SaPIdCMUdCaPa8LaPIdKMUdKaPa8JaPId3MUd3aPayaPIdaMUdaaPagaPId8KMUd8KaPa8MaPIdyMUdyaCcxfhCaKcifgKad6mbkcbh8AaehXincbhCinaYaeaCc:81jjbfydbgLa8AfcdtfydbgAfRbbhPdndnaYaXaCfydbgQfRbbgKc99fcFeGcpe0mbaPceSmbaPcd9hmekdnaKcufcFeGce0mbaEaQcdtfydbaA9hmekdnaPcufcFeGce0mba3aAcdtfydbaQ9hmekdnaKcv2aPfc:G1jjbfRbbTmbaHaAcdtfydbaHaQcdtfydb0mekJbbacJbbjZaPceSEh8MaKceShkaeaLcdtc:81jjbfydba8AfcdtfydbhLdnamaAcx2fgPcwfIdbamaQcx2fgKcwfIdbg8K:tg8La8LNaPIdbaKIdbg8P:tg8Ja8JNaPclfIdbaKclfIdbg8N:tgyayNMM:rggJbbbb9ETmba8Lag:vh8Layag:vhya8Jag:vh8JkJbbaca8MakEh8SdnamaLcx2fgPIdwa8K:tg8Ma8La8Ma8LNaPIdba8P:tgRa8JNayaPIdla8N:tg8RNMMgIN:tg8Ma8MNaRa8JaIN:tg8La8LNa8RayaIN:tg8Ja8JNMM:rgyJbbbb9ETmba8May:vh8Ma8Jay:vh8Ja8Lay:vh8LkazaHaQcdtfydbc8S2fgPa8La8SagNgya8LNNgIaPIdbMUdbaPa8Jaya8JNg8SNgRaPIdlMUdlaPa8Maya8MNggNg8RaPIdwMUdwaPa8Sa8LNg8SaPIdxMUdxaPaga8LNg8UaPIdzMUdzaPaga8JNg8VaPIdCMUdCaPa8Laya8Ma8KNa8La8PNa8Na8JNMM:mg8KNggNg8LaPIdKMUdKaPa8JagNg8JaPId3MUd3aPa8MagNg8MaPIdaMUdaaPaga8KNggaPId8KMUd8KaPayaPIdyMUdyazaHaAcdtfydbc8S2fgPaIaPIdbMUdbaPaRaPIdlMUdlaPa8RaPIdwMUdwaPa8SaPIdxMUdxaPa8UaPIdzMUdzaPa8VaPIdCMUdCaPa8LaPIdKMUdKaPa8JaPId3MUd3aPa8MaPIdaMUdaaPagaPId8KMUd8KaPayaPIdyMUdykaCclfgCcx9hmbkaXcxfhXa8Acifg8Aad6mbkkdnabaeSmbabaeadcdtz:hjjjb8Akcuadcx2adc;v:Q;v:Qe0Ecbyd;S1jjbHjjjjbbhaaqcKfaqyd94gPcdtfaaBdbaqaPcefBd94cuadcdtadcFFFFi0Ecbyd;S1jjbHjjjjbbh5aqcKfaqyd94gPcdtfa5BdbaqaPcefBd94axcbyd;S1jjbHjjjjbbhiaqcKfaqyd94gPcdtfaiBdbaqaPcefBd94alcbyd;S1jjbHjjjjbbh8WaqcKfaqyd94gPcdtfa8WBdbaqaPcefBd94JbbbbhRdnadao9nmbararNh8Saacwfh8Xaqydzh8Yaqydxh8Zaqydwh80JbbbbhRinaqcwfabadgsalaHz:cjjjbcbhhabhXcbhkincbhPindnaHaXaPfydbgAcdtgefydbgKaHabaPc:81jjbfydbakfcdtfydbgCcdtfydbg8ASmbaYaCfRbbgLcv2aYaAfRbbgQfc;q1jjbfRbbgdaQcv2aLfg8Ec;q1jjbfRbbg8FVcFeGTmbdna8Ec:G1jjbfRbbTmba8AaK0mekdnaQaL9hmbaQcufcFeGce0mbaEaefydbaC9hmekaaahcx2fgQaCaAa8FcFeGgKEBdlaQaAaCaKEBdbaQada8FGcFeGcb9hBdwahcefhhkaPclfgPcx9hmbkaXcxfhXakcifgkas6mbkdndnahTmbaahCahh8AinaCcwfgLJbbbbJbbjZazaHaCydbgAcdtfydbc8S2fgPIdyg8L:va8LJbbbb9BEaPIdwamaCclfgeydbgQcx2fgKcwfIdbg8LNaPIdzaKIdbg8JNaPIdaMg8Ma8MMMa8LNaPIdlaKclfIdbg8MNaPIdCa8LNaPId3Mg8La8LMMa8MNaPIdba8JNaPIdxa8MNaPIdKMg8La8LMMa8JNaPId8KMMM:lNgyJbbbbJbbjZazaHaQaAaLydbgKEgLcdtfydbc8S2fgPIdyg8L:va8LJbbbb9BEaPIdwamaAaQaKEgXcx2fgKcwfIdbg8LNaPIdzaKIdbg8JNaPIdaMg8Ma8MMMa8LNaPIdlaKclfIdbg8MNaPIdCa8LNaPId3Mg8La8LMMa8MNaPIdba8JNaPIdxa8MNaPIdKMg8La8LMMa8JNaPId8KMMM:lNg8Laya8L9FgPEUdbaeaQaXaPEBdbaCaAaLaPEBdbaCcxfhCa8Acufg8Ambkaqcjefcbcj;abz1jjjb8Aa8XhPahhCinaqcjefaPydbcO4c;8ZGfgAaAydbcefBdbaPcxfhPaCcufgCmbkcbhPcbhCinaqcjefaPfgAydbhQaAaCBdbaQaCfhCaPclfgPcj;ab9hmbkcbhPa8XhCinaqcjefaCydbcO4c;8ZGfgAaAydbgAcefBdba5aAcdtfaPBdbaCcxfhCahaPcefgP9hmbkasao9RgAci9Uh81dnalTmbcbhPaihCinaCaPBdbaCclfhCalaPcefgP9hmbkkcbhBa8Wcbalz1jjjbh83aAcO9UhUa81ce4h85cbh86cbhkdninaaa5akcdtfydbcx2fgXIdwg8Ja8S9Emea86a819pmeJFFuuh8Ldna85ah9pmbaaa5a85cdtfydbcx2fIdwJbb;aZNh8Lkdna8Ja8L9ETmba86aU0mdkdna83aHaXydlg87cdtg88fydbgAfg89Rbba83aHaXydbgecdtg8:fydbgZfgnRbbVmbdna80aZcdtgPfydbgQTmba8Ya8ZaPfydbcitfhPamaAcx2fg8Ecwfhda8EclfhxamaZcx2fg8Fcwfhva8FclfhwcbhCcehLdnindnaiaPydbcdtfydbgKaASmbaiaPclfydbcdtfydbg8AaASmbama8Acx2fg8AIdbamaKcx2fgKIdbg8M:tg8LawIdbaKclfIdbgy:tggNa8FIdba8M:tg8Ka8AclfIdbay:tg8JN:ta8LaxIdbay:tg8PNa8EIdba8M:tg8Na8JN:tNa8JavIdbaKcwfIdbgy:tgINaga8AcwfIdbay:tg8MN:ta8JadIdbay:tgyNa8Pa8MN:tNa8Ma8KNaIa8LN:ta8Ma8NNaya8LN:tNMMJbbbb9DmdkaPcwfhPaCcefgCaQ6hLaQaC9hmbkkaLceGTmba85cefh85xekaXcwfhQazaAc8S2fgPazaZc8S2fgCIdbaPIdbMUdbaPaCIdlaPIdlMUdlaPaCIdwaPIdwMUdwaPaCIdxaPIdxMUdxaPaCIdzaPIdzMUdzaPaCIdCaPIdCMUdCaPaCIdKaPIdKMUdKaPaCId3aPId3MUd3aPaCIdaaPIdaMUdaaPaCId8KaPId8KMUd8KaPaCIdyaPIdyMUdydndndndnaYaefgCRbbc9:fPdebdkaehPinaiaPcdtgPfaABdbaOaPfydbgPae9hmbxikkaOa88fydbhPaOa8:fydbheaia8:fa87BdbaPh87kaiaecdtfa87Bdbkance86bba89ce86bbaQIdbg8LaRaRa8L9DEhRaBcefhBcecdaCRbbceSEa86fh86kakcefgkah9hmbkkaBTmbdnalTmbcbhCaEhPindnaPydbgAcuSmbdnaCaiaAcdtgQfydbgA9hmbaEaQfydbhAkaPaABdbkaPclfhPalaCcefgC9hmbkcbhCa3hPindnaPydbgAcuSmbdnaCaiaAcdtgQfydbgA9hmba3aQfydbhAkaPaABdbkaPclfhPalaCcefgC9hmbkkcbhdabhPcbhKindnaiaPydbcdtfydbgCaiaPclfydbcdtfydbgASmbaCaiaPcwfydbcdtfydbgQSmbaAaQSmbabadcdtfg8AaCBdba8AclfaABdba8AcwfaQBdbadcifhdkaPcxfhPaKcifgKas9pmdxbkkashdxdkadao0mbkkdnaDTmbaDaR:rUdbkaqyd94gPcdtaqcKffc98fhHdninaPTmeaHydbcbyd;W1jjbH:bjjjbbaHc98fhHaPcufhPxbkkaqcj;bbf8Kjjjjbadk;pleouabydbcbaicdtz1jjjb8Aadci9UhvdnadTmbabydbhodnalTmbaehradhwinaoalarydbcdtfydbcdtfgDaDydbcefBdbarclfhrawcufgwmbxdkkaehradhwinaoarydbcdtfgDaDydbcefBdbarclfhrawcufgwmbkkdnaiTmbabydbhrabydlhwcbhDaihoinawaDBdbawclfhwarydbaDfhDarclfhraocufgombkkdnadci6mbavceavce0EhqabydlhvabydwhrinaecwfydbhwaeclfydbhDaeydbhodnalTmbalawcdtfydbhwalaDcdtfydbhDalaocdtfydbhokaravaocdtfgdydbcitfaDBdbaradydbcitfawBdladadydbcefBdbaravaDcdtfgdydbcitfawBdbaradydbcitfaoBdladadydbcefBdbaravawcdtfgwydbcitfaoBdbarawydbcitfaDBdlawawydbcefBdbaecxfheaqcufgqmbkkdnaiTmbabydlhrabydbhwinararydbawydb9RBdbawclfhwarclfhraicufgimbkkk:3ldouv998Jjjjjbca9Rglczfcwfcbyd11jjbBdbalcb8Pdj1jjb83izalcwfcbydN1jjbBdbalcb8Pd:m1jjb83ibdnadTmbaicd4hvdnabTmbavcdthocbhraehwinabarcx2fgiaearav2cdtfgDIdbUdbaiaDIdlUdlaiaDIdwUdwcbhiinalczfaifgDawaifIdbgqaDIdbgkakaq9EEUdbalaifgDaqaDIdbgkakaq9DEUdbaiclfgicx9hmbkawaofhwarcefgrad9hmbxdkkavcdthrcbhwincbhiinalczfaifgDaeaifIdbgqaDIdbgkakaq9EEUdbalaifgDaqaDIdbgkakaq9DEUdbaiclfgicx9hmbkaearfheawcefgwad9hmbkkalIdbalIdzgk:tJbbbb:xgqalIdlalIdCgx:tgmamaq9DEgqalIdwalIdKgm:tgPaPaq9DEhPdnabTmbadTmbJbbbbJbbjZaP:vaPJbbbb9BEhqinabaqabIdbak:tNUdbabclfgiaqaiIdbax:tNUdbabcwfgiaqaiIdbam:tNUdbabcxfhbadcufgdmbkkaPk:Qdidui99ducbhi8Jjjjjbca9Rglczfcwfcbyd11jjbBdbalcb8Pdj1jjb83izalcwfcbydN1jjbBdbalcb8Pd:m1jjb83ibdndnaembJbbjFhvJbbjFhoJbbjFhrxekadcd4cdthwincbhdinalczfadfgDabadfIdbgoaDIdbgrarao9EEUdbaladfgDaoaDIdbgrarao9DEUdbadclfgdcx9hmbkabawfhbaicefgiae9hmbkalIdwalIdK:thralIdlalIdC:thoalIdbalIdz:thvkavJbbbb:xgvaoaoav9DEgoararao9DEk9DeeuabcFeaicdtz1jjjbhlcbhidnadTmbindnalaeydbcdtfgbydbcu9hmbabaiBdbaicefhikaeclfheadcufgdmbkkaik9teiucbcbyd;01jjbgeabcifc98GfgbBd;01jjbdndnabZbcztgd9nmbcuhiabad9RcFFifcz4nbcuSmekaehikaik;LeeeudndnaeabVciGTmbabhixekdndnadcz9pmbabhixekabhiinaiaeydbBdbaiclfaeclfydbBdbaicwfaecwfydbBdbaicxfaecxfydbBdbaiczfhiaeczfheadc9Wfgdcs0mbkkadcl6mbinaiaeydbBdbaeclfheaiclfhiadc98fgdci0mbkkdnadTmbinaiaeRbb86bbaicefhiaecefheadcufgdmbkkabk;aeedudndnabciGTmbabhixekaecFeGc:b:c:ew2hldndnadcz9pmbabhixekabhiinaialBdbaicxfalBdbaicwfalBdbaiclfalBdbaiczfhiadc9Wfgdcs0mbkkadcl6mbinaialBdbaiclfhiadc98fgdci0mbkkdnadTmbinaiae86bbaicefhiadcufgdmbkkabk9teiucbcbyd;01jjbgeabcrfc94GfgbBd;01jjbdndnabZbcztgd9nmbcuhiabad9RcFFifcz4nbcuSmekaehikaik9:eiuZbhedndncbyd;01jjbgdaecztgi9nmbcuheadai9RcFFifcz4nbcuSmekadhekcbabae9Rcifc98Gcbyd;01jjbfgdBd;01jjbdnadZbcztge9nmbadae9RcFFifcz4nb8Akk6eiucbhidnadTmbdninabRbbglaeRbbgv9hmeaecefheabcefhbadcufgdmbxdkkalav9Rhikaikk:cedbcjwk9PFFuuFFuuFFuuFFuFFFuFFFuFbbbbbbbbeeebeebebbeeebebbbbbebebbbbbebbbdbbbbbbbbbbbbbbbeeeeebebbbbbebbbbbeebbbbbbc;Swkxebbbdbbbj9Kbb";

	var wasmpack = new Uint8Array([32,0,65,2,1,106,34,33,3,128,11,4,13,64,6,253,10,7,15,116,127,5,8,12,40,16,19,54,20,9,27,255,113,17,42,67,24,23,146,148,18,14,22,45,70,69,56,114,101,21,25,63,75,136,108,28,118,29,73,115]);

	if (typeof WebAssembly !== 'object') {
		return {
			supported: false,
		};
	}

	var instance;

	var ready =
		WebAssembly.instantiate(unpack(wasm), {})
		.then(function(result) {
			instance = result.instance;
			instance.exports.__wasm_call_ctors();
		});

	function unpack(data) {
		var result = new Uint8Array(data.length);
		for (var i = 0; i < data.length; ++i) {
			var ch = data.charCodeAt(i);
			result[i] = ch > 96 ? ch - 97 : ch > 64 ? ch - 39 : ch + 4;
		}
		var write = 0;
		for (var i = 0; i < data.length; ++i) {
			result[write++] = (result[i] < 60) ? wasmpack[result[i]] : (result[i] - 60) * 64 + result[++i];
		}
		return result.buffer.slice(0, write);
	}

	function assert(cond) {
		if (!cond) {
			throw new Error("Assertion failed");
		}
	}

	function bytes(view) {
		return new Uint8Array(view.buffer, view.byteOffset, view.byteLength);
	}

	function reorder(indices, vertices) {
		var sbrk = instance.exports.sbrk;
		var ip = sbrk(indices.length * 4);
		var rp = sbrk(vertices * 4);
		var heap = new Uint8Array(instance.exports.memory.buffer);
		var indices8 = bytes(indices);
		heap.set(indices8, ip);
		var unique = instance.exports.meshopt_optimizeVertexFetchRemap(rp, ip, indices.length, vertices);
		// heap may have grown
		heap = new Uint8Array(instance.exports.memory.buffer);
		var remap = new Uint32Array(vertices);
		new Uint8Array(remap.buffer).set(heap.subarray(rp, rp + vertices * 4));
		indices8.set(heap.subarray(ip, ip + indices.length * 4));
		sbrk(ip - sbrk(0));

		for (var i = 0; i < indices.length; ++i)
			indices[i] = remap[indices[i]];

		return [remap, unique];
	}

	function maxindex(source) {
		var result = 0;
		for (var i = 0; i < source.length; ++i) {
			var index = source[i];
			result = result < index ? index : result;
		}
		return result;
	}

	function simplify(fun, indices, index_count, vertex_positions, vertex_count, vertex_positions_stride, target_index_count, target_error, options) {
		var sbrk = instance.exports.sbrk;
		var te = sbrk(4);
		var ti = sbrk(index_count * 4);
		var sp = sbrk(vertex_count * vertex_positions_stride);
		var si = sbrk(index_count * 4);
		var heap = new Uint8Array(instance.exports.memory.buffer);
		heap.set(bytes(vertex_positions), sp);
		heap.set(bytes(indices), si);
		var result = fun(ti, si, index_count, sp, vertex_count, vertex_positions_stride, target_index_count, target_error, options, te);
		// heap may have grown
		heap = new Uint8Array(instance.exports.memory.buffer);
		var target = new Uint32Array(result);
		bytes(target).set(heap.subarray(ti, ti + result * 4));
		var error = new Float32Array(1);
		bytes(error).set(heap.subarray(te, te + 4));
		sbrk(te - sbrk(0));
		return [target, error[0]];
	}

	function simplifyScale(fun, vertex_positions, vertex_count, vertex_positions_stride) {
		var sbrk = instance.exports.sbrk;
		var sp = sbrk(vertex_count * vertex_positions_stride);
		var heap = new Uint8Array(instance.exports.memory.buffer);
		heap.set(bytes(vertex_positions), sp);
		var result = fun(sp, vertex_count, vertex_positions_stride);
		sbrk(sp - sbrk(0));
		return result;
	}

	var simplifyOptions = {
		LockBorder: 1,
	};

	return {
		ready: ready,
		supported: true,

		compactMesh: function(indices) {
			assert(indices instanceof Uint32Array || indices instanceof Int32Array || indices instanceof Uint16Array || indices instanceof Int16Array);
			assert(indices.length % 3 == 0);

			var indices32 = indices.BYTES_PER_ELEMENT == 4 ? indices : new Uint32Array(indices);
			return reorder(indices32, maxindex(indices) + 1);
		},

		simplify: function(indices, vertex_positions, vertex_positions_stride, target_index_count, target_error, flags) {
			assert(indices instanceof Uint32Array || indices instanceof Int32Array || indices instanceof Uint16Array || indices instanceof Int16Array);
			assert(indices.length % 3 == 0);
			assert(vertex_positions instanceof Float32Array);
			assert(vertex_positions.length % vertex_positions_stride == 0);
			assert(vertex_positions_stride >= 3);
			assert(target_index_count % 3 == 0);

			var options = 0;
			for (var i = 0; i < (flags ? flags.length : 0); ++i) {
				options |= simplifyOptions[flags[i]];
			}

			var indices32 = indices.BYTES_PER_ELEMENT == 4 ? indices : new Uint32Array(indices);
			var result = simplify(instance.exports.meshopt_simplify, indices32, indices.length, vertex_positions, vertex_positions.length, vertex_positions_stride * 4, target_index_count, target_error, options);
			result[0] = (indices instanceof Uint32Array) ? result[0] : new indices.constructor(result[0]);

			return result;
		},

		getScale: function(vertex_positions, vertex_positions_stride) {
			assert(vertex_positions instanceof Float32Array);
			assert(vertex_positions.length % vertex_positions_stride == 0);

			return simplifyScale(instance.exports.meshopt_simplifyScale, vertex_positions, vertex_positions.length, vertex_positions_stride * 4);
		},
	};
})();

// UMD-style export for MeshoptSimplifier
if (typeof exports === 'object' && typeof module === 'object')
	module.exports = MeshoptSimplifier;
else if (typeof define === 'function' && define['amd'])
	define([], function() {
		return MeshoptSimplifier;
	});
else if (typeof exports === 'object')
	exports["MeshoptSimplifier"] = MeshoptSimplifier;
else
	(typeof self !== 'undefined' ? self : this).MeshoptSimplifier = MeshoptSimplifier;
