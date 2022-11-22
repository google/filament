// This file is part of meshoptimizer library and is distributed under the terms of MIT License.
// Copyright (C) 2016-2022, by Arseny Kapoulkine (arseny.kapoulkine@gmail.com)
var MeshoptEncoder = (function() {
	"use strict";

	// Built with clang version 14.0.4
	// Built from meshoptimizer 0.18
	var wasm = "b9H79Tebbbenq9Geueu9Geub9Gbb9Gvuuuuueu9Gduueu9Gluuuueu9Gvuuuuub9Gluuuub9Giuuueu9Gd99ue99i8AYdilevlevlooooorrvbwwbeDDDlve9Weiiviebeoweuec:G;kekr;qiHo9TW9T9VV95dbH9F9F939H79T9F9J9H229F9Jt9VV7bb8A9TW79O9V9Wt9FW9U9J9V9KW9wWVtW949c919M9MWVbe8F9TW79O9V9Wt9FW9U9J9V9KW9wWVtW949c919M9MWV9c9V919U9KbdE9TW79O9V9Wt9FW9U9J9V9KW9wWVtW949wWV79P9V9UbiY9TW79O9V9Wt9FW9U9J9V9KW69U9KW949c919M9MWVbl8E9TW79O9V9Wt9FW9U9J9V9KW69U9KW949c919M9MWV9c9V919U9Kbv8A9TW79O9V9Wt9FW9U9J9V9KW69U9KW949wWV79P9V9UboE9TW79O9V9Wt9FW9U9J9V9KW69U9KW949tWG91W9U9JWbra9TW79O9V9Wt9FW9U9J9V9KW69U9KW949tWG91W9U9JW9c9V919U9KbwL9TW79O9V9Wt9FW9U9J9V9KWS9P2tWV9p9JtbDK9TW79O9V9Wt9FW9U9J9V9KWS9P2tWV9r919HtbqL9TW79O9V9Wt9FW9U9J9V9KWS9P2tWVT949WbkE9TW79O9V9Wt9F9V9Wt9P9T9P96W9wWVtW94J9H9J9OWbPa9TW79O9V9Wt9F9V9Wt9P9T9P96W9wWVtW94J9H9J9OW9ttV9P9Wbsa9TW79O9V9Wt9F9V9Wt9P9T9P96W9wWVtW94SWt9J9O9sW9T9H9Wbzl79IV9RbHDwebcekdXCq:1TYdbk;HkeYu8Jjjjjbcjo9Rgv8Kjjjjbcbhodnalcefae0mbabcbRb:q:kjjbc:GeV86bbavcjdfcbcjdz:tjjjb8AdnaiTmbavcjdfadalzMjjjb8Akabaefhrabcefhwavalfcbcbcjdal9RalcFe0Ez:tjjjb8AavavcjdfalzMjjjbhDcj;abal9UhedndnalTmbaec;WFbGgecjdaecjd6Ehqcbhkinakai9pmeaDcjlfcbcjdz:tjjjb8Aaqaiak9Rakaqfai6Egxcsfgecl4cifcd4hmadakal2fhPdndndndndnaec9WGgsTmbcbhzcehHaPhOawhAxekdnaxTmbcbhAcehHaPhCinaDaAfRbbhXaChecbhoinaDcjlfaofaeRbbgQaX9RgXcetaXcKtcK91cr4786bbaealfheaQhXaocefgoax9hmbkaraw9Ram6miawcbamz:tjjjbgeTmiaeamfhwaCcefhCaAcefgAal6hHaAal9hmbxvkkaraw9Ram6moawcbamz:tjjjb8AceheinawgXamfhwalaegoSmldnaraw9Ram6mbaocefheawcbamz:tjjjb8AaXmekkcbhwaoal6moxikindnaxTmbaDazfRbbhXcbheaOhoinaDcjlfaefaoRbbgQaX9RgXcetaXcKtcK91cr4786bbaoalfhoaQhXaecefgeax9hmbkkaraA9Ram6mearaAcbamz:tjjjbgLamfgw9RcK6mdcbhKaDcjlfhCinaDcjlfaKfhYcwh8AczhAcehQindndnaQce9hmbcuhoaYRbbmecbhodninaogecsSmeaecefhoaCaefcefRbbTmbkkcucbaecs6EhoxekaQcetc;:FFFeGhocuaQtcu7cFeGhXcbheinaoaXaCaefRbb9nfhoaecefgecz9hmbkkaoaAaoaA6geEhAaQa8AaeEh8AaQcetgQcw6mbkdndndndna8AcufPdiebkaLaKco4fgeaeRbbcdcia8AclSEaKci4coGtV86bba8Acw9hmeawaY8Pbb83bbawcwfaYcwf8Pbb83bbawczfhwxdkaLaKco4fgeaeRbbceaKci4coGtV86bbkdncwa8A9TgEmbinawcb86bbawcefhwxbkkcua8Atcu7hQcbh3aCh5ina5heaEhXcbhoinaeRbbgAaQaAaQcFeGgY6EaocFeGa8AtVhoaecefheaXcufgXmbkawao86bba5aEfh5awcefhwa3aEfg3cz6mbkcbheindnaCaefRbbgoaY6mbawao86bbawcefhwkaecefgecz9hmbkkdnaKczfgKas9pmbaCczfhCaraw9RcL0mekkaKas6meawTmeaOcefhOazcefgzal6hHawhAazalSmixbkkcbhwaHceGTmexlkcbhwaHceGmikaDaPaxcufal2falzMjjjb8Aaxakfhkawmbkcbhoxdkcbhoaraw9Rcaalalca6E6mednalc8F0mbawcbcaal9Rgez:tjjjbaefhwkawaDcjdfalzMjjjbalfab9Rhoxekcbhokavcjof8Kjjjjbaok9heeuaecaaeca0Eabcj;abae9Uc;WFbGgdcjdadcjd6Egdfcufad9Uae2adcl4cifcd4adV2fcefkmbcbabBd:q:kjjbk;yse3u8Jjjjjbc;ae9Rgl8Kjjjjbcbhvdnaici9UgocHfae0mbabcbydN:kjjbgvc;GeV86bbalc;abfcFecjez:tjjjb8AalcUfgr9cu83ibalc8Wfgw9cu83ibalcyfgD9cu83ibalcafgq9cu83ibalcKfgk9cu83ibalczfgx9cu83ibal9cu83iwal9cu83ibabaefc9WfhmabcefgPaofhednaiTmbcmcsavcb9kgsEhzavce9ihHcbhOcbhAcbhCcbhXcbhQindnaeam9nmbcbhvxikaQcufhvadaCcdtfgoydbhLaocwfydbhKaoclfydbhYcbh8Adndninalc;abfavcsGcitfgoydlhEdndndnaoydbgoaL9hmbaEaYSmekdnaoaY9hmbaEaK9hmba8Acefh8AxekaoaK9hmeaEaL9hmea8Acdfh8Aka8Ac870mdaXcufhvada8AciGcx2goc:y1jjbfydbaCfcdtfydbhEadaocN1jjbfydbaCfcdtfydbhKadaoc:q1jjbfydbaCfcdtfydbhLcbhodnindnalavcsGcdtfydbaE9hmbaohYxdkcuhYavcufhvaocefgocz9hmbkkaYcb9kaYaz9iGgvcu7aEaOSgoGh3dndndndndnaYcbcsaoEavEgvcs9hmbaHmbaEaEaAaEcefaASgvEgAcefSmecmcsavEhvkaPava8Acdtc;WeGV86bbavcs9hmeaEaA9Rgvcetavc8F917hvinaeavcFb0crtavcFbGV86bbaecefheavcje6hoavcr4hvaoTmbkaEhAxdkcPhvaPa8AcdtcPV86bbaEhAkavTmbavaz9imekalaXcdtfaEBdbaXcefcsGhXkaOa3fhOalc;abfaQcitfgvaKBdlavaEBdbalc;abfaQcefcsGgvcitfgoaEBdlaoaLBdbavcefhoxikavcufhva8Aclfg8Ac;ab9hmbkkdnadceaKaOScetaYaOSEcx2gvc:q1jjbfydbaCfcdtfydbgLTadavcN1jjbfydbaCfcdtfydbg8AceSGadavc:y1jjbfydbaCfcdtfydbgYcdSGaOcb9hGasGg5ce9hmbar9cu83ibaw9cu83ibaD9cu83ibaq9cu83ibak9cu83ibax9cu83ibal9cu83iwal9cu83ibcbhOkcbhEaXcufgvhodnindnalaocsGcdtfydba8A9hmbaEhKxdkcuhKaocufhoaEcefgEcz9hmbkkcbhodnindnalavcsGcdtfydbaY9hmbaohExdkcuhEavcufhvaocefgocz9hmbkkaOaLaOSg8Efh3dndnaKcm0mbaKcefhKxekcbcsa8Aa3SgvEhKa3avfh3kdndnaEcm0mbaEcefhExekcbcsaYa3SgvEhEa3avfh3kc9:cua8EEh8FaEaKcltVhocbhvdndndninavcj1jjbfRbbaocFeGSmeavcefgvcz9hmbxdkkaLaO9havcm0Va5VmbaPavc;WeV86bbxekaPa8F86bbaeao86bbaecefhekdna8EmbaLaA9Rgvcetavc8F917hvinaeavcFb0gocrtavcFbGV86bbavcr4hvaecefheaombkaLhAkdnaKcs9hmba8AaA9Rgvcetavc8F917hvinaeavcFb0gocrtavcFbGV86bbavcr4hvaecefheaombka8AhAkdnaEcs9hmbaYaA9Rgvcetavc8F917hvinaeavcFb0gocrtavcFbGV86bbavcr4hvaecefheaombkaYhAkalaXcdtfaLBdbaXcefcsGhvdndnaKPzbeeeeeeeeeeeeeebekalavcdtfa8ABdbaXcdfcsGhvkdndnaEPzbeeeeeeeeeeeeeebekalavcdtfaYBdbavcefcsGhvkalc;abfaQcitfgoaLBdlaoa8ABdbalc;abfaQcefcsGcitfgoa8ABdlaoaYBdbalc;abfaQcdfcsGcitfgoaYBdlaoaLBdbaQcifhoavhXa3hOkaPcefhPaocsGhQaCcifgCai6mbkkcbhvaeam0mbcbhvinaeavfavcj1jjbfRbb86bbavcefgvcz9hmbkaeab9Ravfhvkalc;aef8KjjjjbavkZeeucbhddninadcefgdc8F0meceadtae6mbkkadcrfcFeGcr9Uci2cdfabci9U2cHfkmbcbabBdN:kjjbk:Cdewu8Jjjjjbcz9Rhlcbhvdnaicvfae0mbcbhoabcbRbN:kjjbc;qeV86bbal9cb83iwabcefhrabaefc98fhwdnaiTmbcbhDindnaraw6mbcbskadaDcdtfydbgqalcwfaoaqalcwfaocdtfydb9Rgeaec8F91gefae7c507gocdtfgkydb9Rgec8E91c9:Gaecdt7aoVheinaraecFb0gvcrtaecFbGV86bbaecr4hearcefhravmbkakaqBdbaDcefgDai9hmbkkcbhvaraw0mbarcbBbbarab9RclfhvkavkBeeucbhddninadcefgdc8F0meceadtae6mbkkadcwfcFeGcr9Uab2cvfk:dvli99dui99ludnaeTmbcuadcetcuftcu7:Yhvdndncuaicuftcu7:YgoJbbbZMgr:lJbbb9p9DTmbar:Ohwxekcjjjj94hwkcbhicbhDinalclfIdbgrJbbbbJbbjZalIdbgq:lar:lMalcwfIdbgk:lMgr:varJbbbb9BEgrNhxaqarNhralcxfIdbhqdndnakJbbbb9GTmbaxhkxekJbbjZar:l:tgkak:maxJbbbb9GEhkJbbjZax:l:tgxax:marJbbbb9GEhrkdndnaqJbbj:;aqJbbj:;9GEgxJbbjZaxJbbjZ9FEavNJbbbZJbbb:;aqJbbbb9GEMgq:lJbbb9p9DTmbaq:Ohmxekcjjjj94hmkdndnakJbbj:;akJbbj:;9GEgqJbbjZaqJbbjZ9FEaoNJbbbZJbbb:;akJbbbb9GEMgq:lJbbb9p9DTmbaq:OhPxekcjjjj94hPkdndnarJbbj:;arJbbj:;9GEgqJbbjZaqJbbjZ9FEaoNJbbbZJbbb:;arJbbbb9GEMgr:lJbbb9p9DTmbar:Ohsxekcjjjj94hskdndnadcl9hmbabaDfgzas86bbazcifam86bbazcdfaw86bbazcefaP86bbxekabaifgzas87ebazcofam87ebazclfaw87ebazcdfaP87ebkalczfhlaicwfhiaDclfhDaecufgembkkk;klld99eud99eudnaeTmbdndncuaicuftcu7:YgvJbbbZMgo:lJbbb9p9DTmbao:Ohixekcjjjj94hikaic;8FiGhrinabcofcicdalclfIdb:lalIdb:l9EgialcwfIdb:lalaicdtfIdb:l9EEgialcxfIdb:lalaicdtfIdb:l9EEgiarV87ebdndnalaicefciGcdtfIdbJ;Zl:1ZNJbbj:;JbbjZalaicdtfIdbJbbbb9DEgoNgwJbbj:;awJbbj:;9GEgDJbbjZaDJbbjZ9FEavNJbbbZJbbb:;awJbbbb9GEMgw:lJbbb9p9DTmbaw:Ohqxekcjjjj94hqkabaq87ebdndnaoalaicdfciGcdtfIdbJ;Zl:1ZNNgwJbbj:;awJbbj:;9GEgDJbbjZaDJbbjZ9FEavNJbbbZJbbb:;awJbbbb9GEMgw:lJbbb9p9DTmbaw:Ohqxekcjjjj94hqkabcdfaq87ebdndnaoalaicufciGcdtfIdbJ;Zl:1ZNNgoJbbj:;aoJbbj:;9GEgwJbbjZawJbbjZ9FEavNJbbbZJbbb:;aoJbbbb9GEMgo:lJbbb9p9DTmbao:Ohixekcjjjj94hikabclfai87ebabcwfhbalczfhlaecufgembkkk:7ddwue998Jjjjjbcz9Rgv8KjjjjbdnaeTmbadcl6mbceai9Rhoadcd4gdceadce0EhradcdthwcbhDinc:CuhdalhiarhqinaiIdbavcxfz:xjjjb8Aavydxgkadadak9iEhdaiclfhiaqcufgqmbkaoadfgicKthkcbhdcbai9RhxarhiindndnaladfIdbgmaxz:wjjjbJbbbZJbbb:;amJbbbb9GEMgm:lJbbb9p9DTmbam:Ohqxekcjjjj94hqkabadfaqcFFFrGakVBdbadclfhdaicufgimbkabawfhbalawfhlaDcefgDae9hmbkkavczf8Kjjjjbk;TkdCui998Jjjjjbc:qd9Rgv8Kjjjjbavc:Oefcbc;Kbz:tjjjb8AcbhodnadTmbcbhoaiTmbdnabae9hmbavcuadcdtgoadcFFFFi0Ecbyd1:kjjbHjjjjbbgeBd:OeavceBd1daeabaozMjjjb8Akavc:yefcwfcbBdbav9cb83i:yeavc:yefaeadaiavc:Oefz:njjjbcuaicdtgraicFFFFi0Egwcbyd1:kjjbHjjjjbbhoavc:Oefavyd1dgDcdtfaoBdbavaDcefgqBd1daoavyd:yegkarzMjjjbhxavc:Oefaqcdtfadci9Ugmcbyd1:kjjbHjjjjbbgoBdbavaDcdfgrBd1daocbamz:tjjjbhPavc:Oefarcdtfawcbyd1:kjjbHjjjjbbgsBdbavaDcifgqBd1daxhoashrinaralIdbalaoydbgwcwawcw6Ecdtfc;ebfIdbMUdbaoclfhoarclfhraicufgimbkavc:OefaqcdtfcuamcdtadcFFFF970Ecbyd1:kjjbHjjjjbbgqBdbavaDclfBd1ddnadci6mbamceamce0EhiaehoaqhrinarasaoydbcdtfIdbasaoclfydbcdtfIdbMasaocwfydbcdtfIdbMUdbaocxfhoarclfhraicufgimbkkavc;mbfhzavyd:CehHavyd:GehOavhocbhwcbhrcbhAcehCinaohXcihQaearci2gLcdtfgocwfydbhKaoydbhdabaAcx2fgiclfaoclfydbgDBdbaiadBdbaicwfaKBdbaParfce86bbazaKBdwazaDBdlazadBdbaqarcdtfcbBdbdnawTmbcihQaXhiindnaiydbgoadSmbaoaDSmbaoaKSmbazaQcdtfaoBdbaQcefhQkaiclfhiawcufgwmbkkaAcefhAaxadcdtfgoaoydbcufBdbaxaDcdtfgoaoydbcufBdbaxaKcdtfgoaoydbcufBdbcbhwinaOaHaeawaLfcdtfydbcdtgifydbcdtfgKhoakaifgDydbgdhidnadTmbdninaoydbarSmeaoclfhoaicufgiTmdxbkkaoadcdtaKfc98fydbBdbaDaDydbcufBdbkawcefgwci9hmbkdndndnaQTmbcuhrJbbbbhYcbhoinasazaocdtfydbcdtgifgwIdbh8AawalcbaocefgDaocs0EcdtfIdbalaxaifydbgocwaocw6Ecdtfc;ebfIdbMgEUdbdnakaifydbgwTmbaEa8A:thEaOaHaifydbcdtfhoawcdthiinaqaoydbgwcdtfgdaEadIdbMg8AUdba8AaYaYa8A9DgdEhYawaradEhraoclfhoaic98fgimbkkaDhoaDaQ9hmbkarcu9hmekaCam9pmeindnaPaCfRbbmbaChrxdkamaCcefgC9hmbxdkkaQczaQcz6EhwazhoaXhzarcu9hmekkavyd1dhokaocdtavc:Oeffc98fhrdninaoTmearydbcbyd:e:kjjbH:bjjjbbarc98fhraocufhoxbkkavc:qdf8Kjjjjbk;UlevucuaicdtgvaicFFFFi0Egocbyd1:kjjbHjjjjbbhralalyd9GgwcdtfarBdbalawcefBd9GabarBdbaocbyd1:kjjbHjjjjbbhralalyd9GgocdtfarBdbalaocefBd9GabarBdlcuadcdtadcFFFFi0Ecbyd1:kjjbHjjjjbbhralalyd9GgocdtfarBdbalaocefBd9GabarBdwabydbcbavz:tjjjb8Aadci9UhwdnadTmbabydbhoaehladhrinaoalydbcdtfgvavydbcefBdbalclfhlarcufgrmbkkdnaiTmbabydbhlabydlhrcbhvaihoinaravBdbarclfhralydbavfhvalclfhlaocufgombkkdnadci6mbawceawce0EhDabydlhrabydwhvcbhlinaecwfydbhoaeclfydbhdaraeydbcdtfgwawydbgwcefBdbavawcdtfalBdbaradcdtfgdadydbgdcefBdbavadcdtfalBdbaraocdtfgoaoydbgocefBdbavaocdtfalBdbaecxfheaDalcefgl9hmbkkdnaiTmbabydlheabydbhlinaeaeydbalydb9RBdbalclfhlaeclfheaicufgimbkkkQbabaeadaic:01jjbz:mjjjbkQbabaeadaic:C:jjjbz:mjjjbk9DeeuabcFeaicdtz:tjjjbhlcbhidnadTmbindnalaeydbcdtfgbydbcu9hmbabaiBdbaicefhikaeclfheadcufgdmbkkaik9teiucbcbyd:m:kjjbgeabcifc98GfgbBd:m:kjjbdndnabZbcztgd9nmbcuhiabad9RcFFifcz4nbcuSmekaehikaik;LeeeudndnaeabVciGTmbabhixekdndnadcz9pmbabhixekabhiinaiaeydbBdbaiclfaeclfydbBdbaicwfaecwfydbBdbaicxfaecxfydbBdbaiczfhiaeczfheadc9Wfgdcs0mbkkadcl6mbinaiaeydbBdbaeclfheaiclfhiadc98fgdci0mbkkdnadTmbinaiaeRbb86bbaicefhiaecefheadcufgdmbkkabk;aeedudndnabciGTmbabhixekaecFeGc:b:c:ew2hldndnadcz9pmbabhixekabhiinaialBdbaicxfalBdbaicwfalBdbaiclfalBdbaiczfhiadc9Wfgdcs0mbkkadcl6mbinaialBdbaiclfhiadc98fgdci0mbkkdnadTmbinaiae86bbaicefhiadcufgdmbkkabk9teiucbcbyd:m:kjjbgeabcrfc94GfgbBd:m:kjjbdndnabZbcztgd9nmbcuhiabad9RcFFifcz4nbcuSmekaehikaik9:eiuZbhedndncbyd:m:kjjbgdaecztgi9nmbcuheadai9RcFFifcz4nbcuSmekadhekcbabae9Rcifc98Gcbyd:m:kjjbfgdBd:m:kjjbdnadZbcztge9nmbadae9RcFFifcz4nb8Akkxbabaez:yjjjbk:beeiudnab:8gdcL4gicFeGglcFeSmbdnalmbdnabJbbbb9CmbaecbBdbabskabJbbj9FNaez:xjjjbhbaeaeydbcnfBdbabskaeaicFeGc:cufBdbadcFFF:d94Gcjjj;4iV::hbkabk:Gebdndnaecje9imbabJbbbuNhbdnaecFe9pmbaec:bufhexdkabJbbbuNhbaecpdaecpd6Ec:c9:fhexekaec:bu9kmbabJbbjxNhbdnaec:B9:9nmbaec;MbfhexekabJbbjxNhbaec:299aec:2990Ec;mefhekabaecLtcjjj;8if::Nkk:Eddbcjwk:edb4:h9w9N94:P:gW:j9O:ye9Pbbbbbbebbbdbbbebbbdbbbbbbbdbbbbbbbebbbbbbb:l29hZ;69:9kZ;N;76Z;rg97Z;z;o9xZ8J;B85Z;:;u9yZ;b;k9HZ:2;Z9DZ9e:l9mZ59A8KZ:r;T3Z:A:zYZ79OHZ;j4::8::Y:D9V8:bbbb9s:49:Z8R:hBZ9M9M;M8:L;z;o8:;8:PG89q;x:J878R:hQ8::M:B;e87bbbbbbjZbbjZbbjZ:E;V;N8::Y:DsZ9i;H;68:xd;R8:;h0838:;W:NoZbbbb:WV9O8:uf888:9i;H;68:9c9G;L89;n;m9m89;D8Ko8:bbbbf:8tZ9m836ZS:2AZL;zPZZ818EZ9e:lxZ;U98F8:819E;68:bc:eqkxebbbdbbbaWbb";

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
			instance.exports.meshopt_encodeVertexVersion(0);
			instance.exports.meshopt_encodeIndexVersion(1);
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

	function reorder(indices, vertices, optf) {
		var sbrk = instance.exports.sbrk;
		var ip = sbrk(indices.length * 4);
		var rp = sbrk(vertices * 4);
		var heap = new Uint8Array(instance.exports.memory.buffer);
		var indices8 = bytes(indices);
		heap.set(indices8, ip);
		if (optf) {
			optf(ip, ip, indices.length, vertices);
		}
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

	function encode(fun, bound, source, count, size) {
		var sbrk = instance.exports.sbrk;
		var tp = sbrk(bound);
		var sp = sbrk(count * size);
		var heap = new Uint8Array(instance.exports.memory.buffer);
		heap.set(bytes(source), sp);
		var res = fun(tp, bound, sp, count, size);
		var target = new Uint8Array(res);
		target.set(heap.subarray(tp, tp + res));
		sbrk(tp - sbrk(0));
		return target;
	}

	function maxindex(source) {
		var result = 0;
		for (var i = 0; i < source.length; ++i) {
			var index = source[i];
			result = result < index ? index : result;
		}
		return result;
	}

	function index32(source, size) {
		assert(size == 2 || size == 4);
		if (size == 4) {
			return new Uint32Array(source.buffer, source.byteOffset, source.byteLength / 4);
		} else {
			var view = new Uint16Array(source.buffer, source.byteOffset, source.byteLength / 2);
			return new Uint32Array(view); // copies each element
		}
	}

	function filter(fun, source, count, stride, bits, insize) {
		var sbrk = instance.exports.sbrk;
		var tp = sbrk(count * stride);
		var sp = sbrk(count * insize);
		var heap = new Uint8Array(instance.exports.memory.buffer);
		heap.set(bytes(source), sp);
		fun(tp, count, stride, bits, sp);
		var target = new Uint8Array(count * stride);
		target.set(heap.subarray(tp, tp + count * stride));
		sbrk(tp - sbrk(0));
		return target;
	}

	return {
		ready: ready,
		supported: true,
		reorderMesh: function(indices, triangles, optsize) {
			var optf = triangles ? (optsize ? instance.exports.meshopt_optimizeVertexCacheStrip : instance.exports.meshopt_optimizeVertexCache) : undefined;
			return reorder(indices, maxindex(indices) + 1, optf);
		},
		encodeVertexBuffer: function(source, count, size) {
			assert(size > 0 && size <= 256);
			assert(size % 4 == 0);
			var bound = instance.exports.meshopt_encodeVertexBufferBound(count, size);
			return encode(instance.exports.meshopt_encodeVertexBuffer, bound, source, count, size);
		},
		encodeIndexBuffer: function(source, count, size) {
			assert(size == 2 || size == 4);
			assert(count % 3 == 0);
			var indices = index32(source, size);
			var bound = instance.exports.meshopt_encodeIndexBufferBound(count, maxindex(indices) + 1);
			return encode(instance.exports.meshopt_encodeIndexBuffer, bound, indices, count, 4);
		},
		encodeIndexSequence: function(source, count, size) {
			assert(size == 2 || size == 4);
			var indices = index32(source, size);
			var bound = instance.exports.meshopt_encodeIndexSequenceBound(count, maxindex(indices) + 1);
			return encode(instance.exports.meshopt_encodeIndexSequence, bound, indices, count, 4);
		},
		encodeGltfBuffer: function(source, count, size, mode) {
			var table = {
				ATTRIBUTES: this.encodeVertexBuffer,
				TRIANGLES: this.encodeIndexBuffer,
				INDICES: this.encodeIndexSequence,
			};
			assert(table[mode]);
			return table[mode](source, count, size);
		},
		encodeFilterOct: function(source, count, stride, bits) {
			assert(stride == 4 || stride == 8);
			assert(bits >= 1 && bits <= 16);
			return filter(instance.exports.meshopt_encodeFilterOct, source, count, stride, bits, 16);
		},
		encodeFilterQuat: function(source, count, stride, bits) {
			assert(stride == 8);
			assert(bits >= 4 && bits <= 16);
			return filter(instance.exports.meshopt_encodeFilterQuat, source, count, stride, bits, 16);
		},
		encodeFilterExp: function(source, count, stride, bits) {
			assert(stride > 0 && stride % 4 == 0);
			assert(bits >= 1 && bits <= 24);
			return filter(instance.exports.meshopt_encodeFilterExp, source, count, stride, bits, stride);
		},
	};
})();

export { MeshoptEncoder };
