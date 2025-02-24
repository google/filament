Documentação Beautiful Soup
============================

.. image:: 6.1.jpg
   :align: right
   :alt: "O Lacaio-Peixe começou tirando debaixo do braço uma grande carta, quase tão grande quanto ele mesmo."


`Beautiful Soup <http://www.crummy.com/software/BeautifulSoup/>`_ é uma biblioteca 
Python de extração de dados de arquivos HTML e XML. Ela funciona com o seu interpretador (parser) favorito
a fim de prover maneiras mais intuitivas de navegar, buscar e modificar uma árvore de análise (parse tree). 
Ela geralmente economiza horas ou dias de trabalho de programadores ao redor do mundo.

Estas instruções ilustram as principais funcionalidades do Beautiful Soup 4
com exemplos. Mostro para o que a biblioteca é indicada, como funciona,
como se usa e como fazer aquilo que você quer e o que fazer quando ela frustra suas
expectativas.

Os exemplos nesta documentação devem funcionar da mesma maneira em Python 2.7 e Python 3.2.

`Você pode estar procurando pela documentação do Beautiful Soup 3
<http://www.crummy.com/software/BeautifulSoup/bs3/documentation.html>`_.
Se está, informo que o Beautiful Soup 3 não está mais sendo desenvolvido, 
e que o Beautiful Soup 4 é o recomendado para todos os novos projetos. 
Se você quiser saber as diferenças entre as versões 3 e 4, veja `Portabilidade de código para BS4`_.

Esta documentação foi traduzida para outros idiomas pelos usuários do Beautiful Soup:

* `这篇文档当然还有中文版. <https://www.crummy.com/software/BeautifulSoup/bs4/doc.zh/>`_
* このページは日本語で利用できます(`外部リンク <http://kondou.com/BS4/>`_)
* `이 문서는 한국어 번역도 가능합니다. <https://www.crummy.com/software/BeautifulSoup/bs4/doc.ko/>`_
* `Эта документация доступна на русском языке. <https://www.crummy.com/software/BeautifulSoup/bs4/doc.ru/>`_

Como conseguir ajuda:
---------------------

Se você tem perguntas sobre o Beautiful Soup ou está com dificuldades,
`envie uma mensagem para nosso grupo de discussão
<https://groups.google.com/forum/?fromgroups#!forum/beautifulsoup>`_. Se o seu
problema envolve a interpretação de um documento HTML, não esqueça de mencionar
:ref:`o que a função diagnose() diz <diagnose>` sobre seu documento.

Início Rápido
=============

Este é o HTML que usarei como exemplo ao longo deste documento
É um trecho de "Alice no País das Maravilhas"::

 html_doc = """
 <html><head><title>The Dormouse's story</title></head>
 <body>
 <p class="title"><b>The Dormouse's story</b></p>

 <p class="story">Once upon a time there were three little sisters; and their names were
 <a href="http://example.com/elsie" class="sister" id="link1">Elsie</a>,
 <a href="http://example.com/lacie" class="sister" id="link2">Lacie</a> and
 <a href="http://example.com/tillie" class="sister" id="link3">Tillie</a>;
 and they lived at the bottom of a well.</p>

 <p class="story">...</p>
 """

Executando o arquivo "three sisters" através do Beautiful Soup, ele nos
retorna um objeto ``BeautifulSoup``, que apresenta o documento como uma estrutura
de dados aninhada::

 from bs4 import BeautifulSoup
 soup = BeautifulSoup(html_doc, 'html.parser')

 print(soup.prettify())
 # <html>
 #  <head>
 #   <title>
 #    The Dormouse's story
 #   </title>
 #  </head>
 #  <body>
 #   <p class="title">
 #    <b>
 #     The Dormouse's story
 #    </b>
 #   </p>
 #   <p class="story">
 #    Once upon a time there were three little sisters; and their names were
 #    <a class="sister" href="http://example.com/elsie" id="link1">
 #     Elsie
 #    </a>
 #    ,
 #    <a class="sister" href="http://example.com/lacie" id="link2">
 #     Lacie
 #    </a>
 #    and
 #    <a class="sister" href="http://example.com/tillie" id="link2">
 #     Tillie
 #    </a>
 #    ; and they lived at the bottom of a well.
 #   </p>
 #   <p class="story">
 #    ...
 #   </p>
 #  </body>
 # </html>

Abaixo verificamos algumas maneiras simples de navegar na estrutura::

 soup.title
 # <title>The Dormouse's story</title>

 soup.title.name
 # u'title'

 soup.title.string
 # u'The Dormouse's story'

 soup.title.parent.name
 # u'head'

 soup.p
 # <p class="title"><b>The Dormouse's story</b></p>

 soup.p['class']
 # u'title'

 soup.a
 # <a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>

 soup.find_all('a')
 # [<a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>,
 #  <a class="sister" href="http://example.com/lacie" id="link2">Lacie</a>,
 #  <a class="sister" href="http://example.com/tillie" id="link3">Tillie</a>]

 soup.find(id="link3")
 # <a class="sister" href="http://example.com/tillie" id="link3">Tillie</a>

Uma tarefa comum é extratir todas as URLs encontradas nas tags <a> de uma página::

 for link in soup.find_all('a'):
     print(link.get('href'))
 # http://example.com/elsie
 # http://example.com/lacie
 # http://example.com/tillie

Outra tarefa comum é extrair todo o texto de uma página::

 print(soup.get_text())
 # The Dormouse's story
 #
 # The Dormouse's story
 #
 # Once upon a time there were three little sisters; and their names were
 # Elsie,
 # Lacie and
 # Tillie;
 # and they lived at the bottom of a well.
 #
 # ...

Isso se parece com o que você precisa? Então vá em frente!

Instalando o Beautiful Soup
===========================

Se você está usando uma versão recente das distribuições Linux Debian ou Ubuntu,
você pode instalar o Beautiful Soup facilmente utilizando o gerenciador de pacotes

:kbd:`$ apt-get install python-bs4` (for Python 2)

:kbd:`$ apt-get install python3-bs4` (for Python 3)

O Beautiful Soup 4 também está publicado no PyPi. Portanto, se
você não conseguir instalá-lo através de seu gerenciador de pacotes, você
pode fazer isso com ``easy_install`` ou ``pip``. O nome do pacote é ``beautifulsoup4``, 
e o mesmo pacote é válido tanto para Python 2 quanto Python 3. Tenha certeza de utilizar 
a versão correta de ``pip`` ou ``easy_install`` para sua versão do Python (estarão 
nomeados como ``pip3`` ou ``easy_install3`` ,respectivamente, se você estiver usando Python 3).


:kbd:`$ easy_install beautifulsoup4`

:kbd:`$ pip install beautifulsoup4`

(O pacote ``BeautifulSoup`` provavelmente `não` é o que você quer. Esta
é a versão anterior, `Beautiful Soup 3`_. Muitos softwares utilizam
BS3, por isso ele ainda está disponível, mas se você está criando algo novo,
você deve instalar o ``beautifulsoup4``.)

Se você não possui o ``easy_install`` ou ``pip`` instalados, você pode fazer 
o download através do tarball do arquivo fonte do Beautiful Soup 4
<http://www.crummy.com/software/BeautifulSoup/download/4.x/>`_ e
instalar através do ``setup.py``.

:kbd:`$ python setup.py install`

Se tudo isso falhar, a licença do Beautiful Soup lhe permite empacotar
toda a biblioteca em sua aplicação. Você pode fazer o download do arquivo
tarball, copiar o diretório ``bs4`` do código-fonte para sua aplicação e
utilizar o Beautiful Soup sem nenhum processo de instalação.

Eu utilizo Python 2.7 e Python 3.2 para desenvolver o Beautiful Soup,
mas ele também funcionará com outras versões recentes.

Problemas após a instalação
---------------------------

O Beautiful Soup é empacotado em Python 2. Quando você o instala utilizando
Python 3 ele é automaticamente convertido para esta versão. Se você não instalar o pacote, o
código não será convertido. Também foi relatado versões erradas sendo instaladas em 
máquinas Windows.

Se você receber um ``ImportError`` "No module named HTMLParser", seu problema
é que você está utilizando o formato de código Python 2 sob Python 3.

Se você receber um ``ImportError`` "No module named html.parser", seu problema
é que você está utilizando o formato de código Python 3 sob Python 2.

Em ambos os casos, sua melhor opção é remover completamente a
instalação do Beautiful Soup do seu sistema (incluindo qualquer diretório
criado quando o tarball foi descompactado) e realizar a instalação novamente.

Se você receber um ``SyntaxError`` "Invalid syntax" na linha 
``ROOT_TAG_NAME = u'[document]'``, você terá que converter o Python 2
em Python 3. Você pode fazer isso instalando o pacote:

:kbd:`$ python3 setup.py install`

ou manualmente executando o script de conversão ``2to3`` no
diretório ``bs4``:

:kbd:`$ 2to3-3.2 -w bs4`

.. _parser-installation:


Instalando um interpretador (parser)
------------------------------------


O Beautiful Soup não só suporta o parser HTML incluído na biblioteca 
padrão do Python como também inúmeros parsers de terceiros.
Um deles é o `parser lxml <http://lxml.de/>`_. Dependendo de sua configuração,
você podera instalar o lxml com algum dos seguintes comandos:

:kbd:`$ apt-get install python-lxml`

:kbd:`$ easy_install lxml`

:kbd:`$ pip install lxml`

Outra alternativa é o parser `html5lib
<http://code.google.com/p/html5lib/>`_ do Python puro, o qual analisa o HTML 
da mesma maneira que o navegador o faz. Dependendo de sua configuração,
você podera instalar o html5lib com algum dos seguintes comandos:

:kbd:`$ apt-get install python-html5lib`

:kbd:`$ easy_install html5lib`

:kbd:`$ pip install html5lib`

Esta tabela resume as vantagens e desvantagens de cada parser:-

+----------------------+--------------------------------------------+--------------------------------+--------------------------+
| Parser               | Uso Padrão                                 | Vantagens                      | Desvantagens             |
+----------------------+--------------------------------------------+--------------------------------+--------------------------+
|  html.parser (puro)  | ``BeautifulSoup(markup, "html.parser")``   | * Baterias inclusas            | * Não tão rápido quanto  |
|                      |                                            | * Velocidade Decente           |   lxml, menos leniente   |
|                      |                                            | * Leniente (Python 2.7.3       |   que html5lib.          |
|                      |                                            |   e 3.2.)                      |                          |
+----------------------+--------------------------------------------+--------------------------------+--------------------------+
|     HTML (lxml)      | ``BeautifulSoup(markup, "lxml")``          | * Muito rápido                 | * Dependencia externa de |
|                      |                                            | * Leniente                     |   C                      |
+----------------------+--------------------------------------------+--------------------------------+--------------------------+
|      XML (lxml)      | ``BeautifulSoup(markup, "lxml-xml")``      | * Muito rápido                 | * Dependência externa de |
|                      | ``BeautifulSoup(markup, "xml")``           | * O único parser XML atualmente|   C                      |
|                      |                                            |   suportado                    |                          |
+----------------------+--------------------------------------------+--------------------------------+--------------------------+
|      html5lib        | ``BeautifulSoup(markup, "html5lib")``      | * Extremamente leniente        | * Muito lento            |
|                      |                                            | * Analisa as páginas da mesma  | * Dependência externa de |
|                      |                                            |   maneira que o navegador o faz|   Python                 |
|                      |                                            | * Cria HTML5 válidos           |                          |
+----------------------+--------------------------------------------+--------------------------------+--------------------------+

Se for possível recomendo que você instale e utilize o lxml pelo desempenho.
Se você está utilizando o Python 2 anterior a 2.7.3 ou uma versão do Python 3
anterior a 3.2.2, é `essencial` que você instale o lxml ou o html5lib. O parser
HTML nativo do Python não é muito bom para versões mais antigas.

Note que se um documento é inválido, diferentes parsers irão gerar
diferentes árvores BeautifulSoup para isso. Veja
:ref:`Diferenças entre os interpretadores (parsers) <differences-between-parsers>`
para detalhes.


Criando a "Sopa"
================

Para analisar um documento, passe-o como argumento dentro de um construtor ``BeautifulSoup``.
Você pode passar este argumento como uma string ou manipulador da função open()::

 from bs4 import BeautifulSoup

 with open("index.html") as fp:
     soup = BeautifulSoup(fp)

 soup = BeautifulSoup("<html>data</html>")

Primeiro, o documento é convertido para Unicode e as entidades HTML
são convertidas para caracteres Unicode::

 BeautifulSoup("Sacr&eacute; bleu!")
 <html><head></head><body>Sacré bleu!</body></html>

O Beautiful Soup então interpreta o documento usando o melhor parser disponível.
Ele irá utilizar um parser HTML ao menos que você indique a ele que utilize um
parser XML. (Veja `Analisando um XML`_.)

Tipos de objetos
================

O Beautiful Soup transforma um documento HTML complexo em uma complexa árvore de objetos Python.
Mas você terá apenas que lidar com quatro `tipos` de objetos: ``Tag``, ``NavigableString``, ``BeautifulSoup``,
e ``Comment``.

.. _Tag:

``Tag``
-------

Um objeto ``Tag``  corresponde a uma tag XML ou HTML do documento original::

 soup = BeautifulSoup('<b class="boldest">Extremely bold</b>')
 tag = soup.b
 type(tag)
 # <class 'bs4.element.Tag'>

As tags possuem muitos atributos e métodos que eu falarei mais sobre em
`Navegando pela árvore`_ e `Buscando na árvore`_. Por agora, as características
mais importantes da tag são seu nome e atributos.

Nome
^^^^

Toda tag possui um nome, acessível através de ``.name``::

 tag.name
 # u'b'

Se você mudar o nome de uma tag, a alteração será refletida em qualquer HTML gerado pelo 
Beautiful Soup::

 tag.name = "blockquote"
 tag
 # <blockquote class="boldest">Extremely bold</blockquote>

Atributos
^^^^^^^^^^
Uma tag pode ter inúmeros atributos. A tag ``<b id="boldest">`` 
possui um atributo "id" que possui o valor "boldest". Você pode 
acessar um atributo de uma tag tratando-a como um dicionário::

 tag['id']
 # u'boldest'

Você pode acessar este dicionário diretamente através de ``.attrs``::

 tag.attrs
 # {u'id': 'boldest'}

Você pode adicionar, remover ou modificar os atributos de uma tag. Novamente, isso pode
ser feito tratando a tag como um dicionário::

 tag['id'] = 'verybold'
 tag['another-attribute'] = 1
 tag
 # <b another-attribute="1" id="verybold"></b>

 del tag['id']
 del tag['another-attribute']
 tag
 # <b></b>

 tag['id']
 # KeyError: 'id'
 print(tag.get('id'))
 # None

.. _multivalue:

Atributos com múltiplos valores
&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

O HTML 4 define alguns atributos que podem ter múltiplos valores. O HTML 5
removeu alguns deles, mas definiu alguns novos. O atributo mais comum
que pode receber múltiplos valores é o ``class`` (ou seja, a tag pode ter mais de uma classe CSS). 
Outros são ``rel``, ``rev``, ``accept-charset``, ``headers``, e ``accesskey``. 
O Beautiful Soup apresenta o(s) valor(es) de um atributo deste tipo como uma lista::

 css_soup = BeautifulSoup('<p class="body"></p>')
 css_soup.p['class']
 # ["body"]
  
 css_soup = BeautifulSoup('<p class="body strikeout"></p>')
 css_soup.p['class']
 # ["body", "strikeout"]

Se um atributo possui mais de um valor, mas não é um atributo
que aceita múltiplos valores conforme definido por qualquer versão do
padrão HTML, o Beautiful Soup retornará como um valor único::

 id_soup = BeautifulSoup('<p id="my id"></p>')
 id_soup.p['id']
 # 'my id'

Quando a tag é transformada novamente em string, os valores do atributo múltiplo
são consolidados::

 rel_soup = BeautifulSoup('<p>Back to the <a rel="index">homepage</a></p>')
 rel_soup.a['rel']
 # ['index']
 rel_soup.a['rel'] = ['index', 'contents']
 print(rel_soup.p)
 # <p>Back to the <a rel="index contents">homepage</a></p>

Você pode desabilitar esta opção passando ``multi_valued_attributes=None`` como argumento
dentro do construtor ``BeautifulSoup`` ::

  no_list_soup = BeautifulSoup('<p class="body strikeout"></p>', 'html', multi_valued_attributes=None)
  no_list_soup.p['class']
  # u'body strikeout'

Você pode utilizar ```get_attribute_list`` para retornar um valor no formato de lista, seja um atributo de
múltiplos valores ou não::

  id_soup.p.get_attribute_list('id')
  # ["my id"]
 
Se você analisar um documento como XML, nenhum atributo será tratado como de múltiplos valores::

 xml_soup = BeautifulSoup('<p class="body strikeout"></p>', 'xml')
 xml_soup.p['class']
 # u'body strikeout'

Novamente, você pode configurar isto usando o argumento ``multi_valued_attributes``::

  class_is_multi= { '*' : 'class'}
  xml_soup = BeautifulSoup('<p class="body strikeout"></p>', 'xml', multi_valued_attributes=class_is_multi)
  xml_soup.p['class']
  # [u'body', u'strikeout']

Você provavelmente não precisará fazer isso, mas se fizer, use os padrões como guia.
Eles implementam as regras descritas na especificação do HTML::

  from bs4.builder import builder_registry
  builder_registry.lookup('html').DEFAULT_CDATA_LIST_ATTRIBUTES

  
``NavigableString``
-------------------

Uma string corresponde a um texto dentro de uma tag.
O Beautiful Soup usa a classe ``NavigableString`` para armazenar este texto::

 tag.string
 # u'Extremely bold'
 type(tag.string)
 # <class 'bs4.element.NavigableString'>

Uma ``NavigableString`` é como uma string Unicode do Python, exceto
que ela também suporta algumas características descritas em `Navegando pela árvore`_ 
e `Buscando na árvore`_. Você pode converter um
``NavigableString`` em uma string Unicode utilizando ``unicode()``::

 unicode_string = unicode(tag.string)
 unicode_string
 # u'Extremely bold'
 type(unicode_string)
 # <type 'unicode'>

Você não pode editar uma string "in place", mas você pode substituir
uma string por outra usando :ref:`replace_with()`::

 tag.string.replace_with("No longer bold")
 tag
 # <blockquote>No longer bold</blockquote>

``NavigableString`` suporta a maior parte das características descritas em 
`Navegando pela árvore`_ e `Buscando na árvore`_, mas não todas elas. 
Em particular, desde que uma string não pode conter de tudo (da maneira que
uma tag pode conter uma string ou outra tag), as strings não suportam os
atributos ``.contents`` ou ``.string`` ou o método ``find()``.

Se você quer utilizar uma ``NavigableString`` fora do Beautiful Soup,
você deve chamar o ``unicode()`` para transformá-la em uma string Unicode Python
padrão. Se você não fizer isso, sua string irá carregar uma referência de toda sua 
árvore Beautiful Soup, mesmo que você já não esteja mais usando ela, o que é um grande
desperdício de memória.

``BeautifulSoup``
-----------------

O objeto ``BeautifulSoup`` em si representa o documento como um todo.
Para maioria dos propósitos, você pode tratá-lo como um objeto :ref:`Tag`.
Isso significa que irá suportar a maioria dos métodos descritos em
`Navegando pela árvore`_ e `Buscando na árvore`_.

Sabendo que o objeto ``BeautifulSoup`` não corresponde a uma tag
HTML ou XML propriamente dita, ele não tem nome e nem atributos. Mas em alguns
casos é útil observar seu ``.name``; então, foi dado o especial
``.name`` "[document]"::

 soup.name
 # u'[document]'

Comentários e outras strings especiais
--------------------------------------

``Tag``, ``NavigableString``, e ``BeautifulSoup`` abrangem quase
tudo o que você encontrará em um arquivo HTML ou XML, mas há alguns
pontos faltando. O único deles que você provavelmente precisará se preocupar
é o comentário::

 markup = "<b><!--Hey, buddy. Want to buy a used parser?--></b>"
 soup = BeautifulSoup(markup)
 comment = soup.b.string
 type(comment)
 # <class 'bs4.element.Comment'>

O objeto ``Comment`` é apenas um tipo especial de ``NavigableString``::

 comment
 # u'Hey, buddy. Want to buy a used parser'

Mas quando aparece como parte de um documento HTML, um ``Comment`` é
exibido com uma formatação especial::

 print(soup.b.prettify())
 # <b>
 #  <!--Hey, buddy. Want to buy a used parser?-->
 # </b>

O Beautiful Soup define classes para qualquer outra coisa que possa
aparecer em um documento XML: ``CData``, ``ProcessingInstruction``,
``Declaration`` e ``Doctype``. Assim como ``Comment``, estas classes 
são subclasses de ``NavigableString`` que adicionam algo a string.
Aqui está um exemplo que substitui o comentário por um bloco CDATA::

 from bs4 import CData
 cdata = CData("A CDATA block")
 comment.replace_with(cdata)

 print(soup.b.prettify())
 # <b>
 #  <![CDATA[A CDATA block]]>
 # </b>


Navegando pela árvore
=====================

Aqui está o documento HTML "Three sisters" novamente::

 html_doc = """
 <html><head><title>The Dormouse's story</title></head>
 <body>
 <p class="title"><b>The Dormouse's story</b></p>

 <p class="story">Once upon a time there were three little sisters; and their names were
 <a href="http://example.com/elsie" class="sister" id="link1">Elsie</a>,
 <a href="http://example.com/lacie" class="sister" id="link2">Lacie</a> and
 <a href="http://example.com/tillie" class="sister" id="link3">Tillie</a>;
 and they lived at the bottom of a well.</p>

 <p class="story">...</p>
 """

 from bs4 import BeautifulSoup
 soup = BeautifulSoup(html_doc, 'html.parser')

Eu usarei este documento como exemplo para mostrar como navegar
de uma parte para outra do documento.

Descendo na Árvore
------------------
As tags podem conter strings e outras tags. Estes elementos são as tags
`filhas` (children). O Beautiful Soup oferece diferentes atributos para 
navegar e iterar sobre as tags filhas.

Note que as strings Beautiful Soup não suportam qualquer destes atributos,
porque uma string não pode ter filhos.

Navegar usando os nomes das tags
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
A maneira mais simples de navegar pela árvore é utilizar
o nome da tag que você quer. Se você quer a tag <head>, 
simplesmente use ``soup.head``::

 soup.head
 # <head><title>The Dormouse's story</title></head>

 soup.title
 # <title>The Dormouse's story</title>

Você pode usar este truque de novo, e de novo, para focar em certa parte da
árvore de análise. Este código retorna a primeira tag <b> abaixo da tag <body>::

 soup.body.b
 # <b>The Dormouse's story</b>

Utilizando o nome da tag como atributo irá lhe retornar apenas a `primeira`
tag com aquele nome::

 soup.a
 # <a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>

Se você precisar retornar `todas` as tags <a>, ou algo mais complicado
que a primeira tag com um certo nome, você precisará utilizar um dos
métodos descritos em `Buscando na árvore`_, como `find_all()`::

 soup.find_all('a')
 # [<a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>,
 #  <a class="sister" href="http://example.com/lacie" id="link2">Lacie</a>,
 #  <a class="sister" href="http://example.com/tillie" id="link3">Tillie</a>]

``.contents`` e ``.children``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

As tags filhas de uma outra tag estão disponíveis em uma lista chamada por ``.contents``::

 head_tag = soup.head
 head_tag
 # <head><title>The Dormouse's story</title></head>

 head_tag.contents
 [<title>The Dormouse's story</title>]

 title_tag = head_tag.contents[0]
 title_tag
 # <title>The Dormouse's story</title>
 title_tag.contents
 # [u'The Dormouse's story']

O objeto ``BeautifulSoup`` em si possui filhos. Neste caso, a tag
<html> é a filha do objeto ``BeautifulSoup``.::

 len(soup.contents)
 # 1
 soup.contents[0].name
 # u'html'

Uma string não possui o atributo ``.contents``, porque ela não pode conter
nada::

 text = title_tag.contents[0]
 text.contents
 # AttributeError: 'NavigableString' object has no attribute 'contents'

Ao invés de retorná-las como uma lista, você pode iterar sobre as
tag's filhas usando o gerador ``.children``::

 for child in title_tag.children:
     print(child)
 # The Dormouse's story

``.descendants``
^^^^^^^^^^^^^^^^

Os atributos ``.contents`` e ``.children`` somente consideram tags que
são `filhas diretas`. Por instância, a tag <head> tem apenas uma tag filha direta, 
a tag <title>::

 head_tag.contents
 # [<title>The Dormouse's story</title>]

Mas a tag <title> em si possui uma filha: a string "The Dormouse's story". 
Existe uma percepção de que esta string também é filha da tag <head>.
O atributo ``.descendants`` permite que você itere sobre `todas`
as tags filhas, recursivamente: suas filhas diretas, as filhas de suas filhas, e assim por diante::

 for child in head_tag.descendants:
     print(child)
 # <title>The Dormouse's story</title>
 # The Dormouse's story

A tag <head> possui apenas uma filha, mas também possui dois `descentendes`: 
a tag <title> e a filha da tag <title>. O objeto ``BeautifulSoup`` possui apenas
uma filha direta (a tag <html>), mas ele possui vários descendentes::

 len(list(soup.children))
 # 1
 len(list(soup.descendants))
 # 25

.. _.string:

``.string``
^^^^^^^^^^^

Se uma tag possui apenas uma filha, e esta filha é uma ``NavigableString``,
esta filha pode ser disponibilizada através de ``.string``::

 title_tag.string
 # u'The Dormouse's story'

Se a filha única de uma tag é outra tag e esta tag possui uma
``.string``, então considera-se que a tag mãe tenha a mesma
``.string`` como sua filha::

 head_tag.contents
 # [<title>The Dormouse's story</title>]

 head_tag.string
 # u'The Dormouse's story'

Se uma tag contém mais de uma coisa, então não fica claro a que
``.string`` deve se referir, portanto ``.string`` será definida como
``None``::

 print(soup.html.string)
 # None

.. _string-generators:

``.strings`` e ``stripped_strings``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Se existe mais de alguma coisa dentro da tag, você pode continuar
olhando apenas as strings. Use o gerador ``.strings``::

 for string in soup.strings:
     print(repr(string))
 # u"The Dormouse's story"
 # u'\n\n'
 # u"The Dormouse's story"
 # u'\n\n'
 # u'Once upon a time there were three little sisters; and their names were\n'
 # u'Elsie'
 # u',\n'
 # u'Lacie'
 # u' and\n'
 # u'Tillie'
 # u';\nand they lived at the bottom of a well.'
 # u'\n\n'
 # u'...'
 # u'\n'

Estas strings tendem a ter muitos espaços em branco, os quais você
pode remover utilizando o gerador ``.stripped_strings`` como alternativa::

 for string in soup.stripped_strings:
     print(repr(string))
 # u"The Dormouse's story"
 # u"The Dormouse's story"
 # u'Once upon a time there were three little sisters; and their names were'
 # u'Elsie'
 # u','
 # u'Lacie'
 # u'and'
 # u'Tillie'
 # u';\nand they lived at the bottom of a well.'
 # u'...'

Aqui, strings formadas inteiramente por espaços em branco serão ignoradas,
e espaços em branco no início e no fim das strings serão removidos.

Subindo na Árvore
-----------------

Continuando a analogia da árvore como "família", toda tag e toda string possuem
`tags mães (parents)`: a tag que as contém.

.. _.parent:

``.parent``
^^^^^^^^^^^

Você pode acessar o elemento mãe com o atributo ``.parent``. No
exemplo "three sisters", a tag <head> é mãe da tag <title>::

 title_tag = soup.title
 title_tag
 # <title>The Dormouse's story</title>
 title_tag.parent
 # <head><title>The Dormouse's story</title></head>

A string de title tem uma mãe: a tag <title> que a contém::

 title_tag.string.parent
 # <title>The Dormouse's story</title>

A tag mãe de todo documento (<html>) é um objeto ``BeautifulSoup`` em si::

 html_tag = soup.html
 type(html_tag.parent)
 # <class 'bs4.BeautifulSoup'>

E o ``.parent`` de um objeto ``BeautifulSoup`` é definido como None::

 print(soup.parent)
 # None

.. _.parents:

``.parents``
^^^^^^^^^^^^
Você pode iterar sobre todos os elementos pais com
``.parents``. Este exemplo usa ``.parents`` para viajar de uma tag <a> 
profunda no documento, para o elemento mais ao topo da árvore do documento::

 link = soup.a
 link
 # <a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>
 for parent in link.parents:
     if parent is None:
         print(parent)
     else:
         print(parent.name)
 # p
 # body
 # html
 # [document]
 # None

Navegando para os lados:
------------------------

Considere um simples documento como este::

 sibling_soup = BeautifulSoup("<a><b>text1</b><c>text2</c></b></a>")
 print(sibling_soup.prettify())
 # <html>
 #  <body>
 #   <a>
 #    <b>
 #     text1
 #    </b>
 #    <c>
 #     text2
 #    </c>
 #   </a>
 #  </body>
 # </html>

A tag <b> e a tag <c> estão no mesmo nível: ambas são filhas diretas 
da mesma tag. Nós podemos chamá-las irmãs (`siblings`).
Quando um documento é pretty-printed, irmãs aparecem no mesmo nível de identação.
Você pode utilizar esta relação nos códigos que você escrever.

``.next_sibling`` e ``.previous_sibling``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Você pode usar ``.next_sibling`` e ``.previous_sibling`` para navegar
entre os elementos da página que estão no mesmo nível da árvore::

 sibling_soup.b.next_sibling
 # <c>text2</c>

 sibling_soup.c.previous_sibling
 # <b>text1</b>

A tag <b> possui ``.next_sibling``, mas não ``.previous_sibling``,
porque não há nada antes da tag <b> `no mesmo nível na árvore`.
Pela mesma razão, a tag <c> possui ``.previous_sibling``
mas não ``.next_sibling``::

 print(sibling_soup.b.previous_sibling)
 # None
 print(sibling_soup.c.next_sibling)
 # None

As strings "text1" e "text2" `não` são irmãs, porque elas não tem a mesma tag mãe::

 sibling_soup.b.string
 # u'text1'

 print(sibling_soup.b.string.next_sibling)
 # None

No mundo real, ``.next_sibling`` ou ``.previous_sibling`` de uma tag
geralmente são strings contendo espaços em branco. Voltando ao documento
"three sisters"::

 <a href="http://example.com/elsie" class="sister" id="link1">Elsie</a>
 <a href="http://example.com/lacie" class="sister" id="link2">Lacie</a>
 <a href="http://example.com/tillie" class="sister" id="link3">Tillie</a>

Você pode pensar que o ``.next_sibling`` da primeira tag <a> será a segunda tag <a>.
Mas na verdade é uma string: a vírgula e um caracter de nova linha (\n) que separam 
a primeira da segunda tag <a>::

 link = soup.a
 link
 # <a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>

 link.next_sibling
 # u',\n'

A segunda tag <a> é, na verdade, a ``.next_sibling`` da vírgula::

 link.next_sibling.next_sibling
 # <a class="sister" href="http://example.com/lacie" id="link2">Lacie</a>

.. _sibling-generators:

``.next_siblings`` e ``.previous_siblings``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Você pode iterar sobre as tag's filhas com ``.next_siblings``
ou ``.previous_siblings``::

 for sibling in soup.a.next_siblings:
     print(repr(sibling))
 # u',\n'
 # <a class="sister" href="http://example.com/lacie" id="link2">Lacie</a>
 # u' and\n'
 # <a class="sister" href="http://example.com/tillie" id="link3">Tillie</a>
 # u'; and they lived at the bottom of a well.'
 # None

 for sibling in soup.find(id="link3").previous_siblings:
     print(repr(sibling))
 # ' and\n'
 # <a class="sister" href="http://example.com/lacie" id="link2">Lacie</a>
 # u',\n'
 # <a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>
 # u'Once upon a time there were three little sisters; and their names were\n'
 # None

Indo e voltando
----------------

Dê uma olhada no início do documento "three sisters"::

 <html><head><title>The Dormouse's story</title></head>
 <p class="title"><b>The Dormouse's story</b></p>

Um parser HTML transforma estas strings em uma série de eventos: "abrir 
uma tag <html>", "abrir uma tag <head>", "abrir uma tag <title>", 
"adicionar uma string", "fechar uma tag <title>,
"abrir uma tag <p>", e daí por diante. O Beautiful Soup oferece ferramentas
para reconstruir a análise inicial do documento.

.. _element-generators:

``.next_element`` e ``.previous_element``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

O atributo ``.next_element`` de uma string ou tag aponta para 
qualquer coisa que tenha sido interpretado posteriormente.
Isso deveria ser o mesmo que ``.next_sibling``, mas é 
drasticamente diferente.

Aqui está a tag <a> final no "three sisters". Sua
``.next_sibling`` é uma string: a conclusão da sentença
que foi interrompida pelo início da tag <a>.::

 last_a_tag = soup.find("a", id="link3")
 last_a_tag
 # <a class="sister" href="http://example.com/tillie" id="link3">Tillie</a>

 last_a_tag.next_sibling
 # '; and they lived at the bottom of a well.'

Mas no ``.next_element`` da tag <a>, o que é analisado imediatamente
depois da tag <a> `não` é o resto da sentença: é a palavra "Tillie".

 last_a_tag.next_element
 # u'Tillie'

Isso porque na marcação original, a palavra "Tillie" apareceu
antes do ponto e virgula. O parser encontrou uma tag <a>, então
a palavra "Tillie", então fechando a tag </a>, então o ponto e vírgula e o
resto da sentença. O ponto e vírgula estão no mesmo nível que a tag <a>,
mas a palavra "Tillie" foi encontrada primeiro.

O atributo ``.previous_element`` é exatamente o oposto de
``.next_element``. Ele aponta para qualquer elemento que
seja analisado antes do respectivo::

 last_a_tag.previous_element
 # u' and\n'
 last_a_tag.previous_element.next_element
 # <a class="sister" href="http://example.com/tillie" id="link3">Tillie</a>

``.next_elements`` e ``.previous_elements``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Você deve ter entendido a idéia agora. Você pode usar estes iteradores
para andar para frente e para atrás no documento quando ele for analisado::

 for element in last_a_tag.next_elements:
     print(repr(element))
 # u'Tillie'
 # u';\nand they lived at the bottom of a well.'
 # u'\n\n'
 # <p class="story">...</p>
 # u'...'
 # u'\n'
 # None

Buscando na árvore
==================

O Beautiful Soup define vários métodos para buscar na árvore que está sendo analisada,
mas eles são todos muito similares. Vou usar a maior parte do tempo para explicar os dois mais
populares métodos: ``find()`` e ``find_all()``. Os outros métodos recebem exatamente
os mesmos argumentos, portanto, vou cobrí-los apenas brevemente.


Mais uma vez, utilizarei o documento "three sisters" como exemplo::

 html_doc = """
 <html><head><title>The Dormouse's story</title></head>
 <body>
 <p class="title"><b>The Dormouse's story</b></p>

 <p class="story">Once upon a time there were three little sisters; and their names were
 <a href="http://example.com/elsie" class="sister" id="link1">Elsie</a>,
 <a href="http://example.com/lacie" class="sister" id="link2">Lacie</a> and
 <a href="http://example.com/tillie" class="sister" id="link3">Tillie</a>;
 and they lived at the bottom of a well.</p>

 <p class="story">...</p>
 """

 from bs4 import BeautifulSoup
 soup = BeautifulSoup(html_doc, 'html.parser')

Utilizando em um filtro um argumento como ``find_all()``, você pode
"dar um zoom" nas partes do documento que você está interessado.

Tipos de filtros
----------------

Antes de entrar em detalhes sobre o ``find_all()`` e métodos similares,
quero mostrar exemplos de diferentes filtros que você pode passar dentro
destes métodos. Estes filtros aparecerão de novo e de novo por toda API 
de pesquisa. Você pode usá-los para realizar filtros baseados nos nomes das tags, 
nos seus atributos, no texto de uma strings ou em alguma combinação entre eles.

.. _uma string:

Uma string
^^^^^^^^^^

O filtro mais simples é uma string. Passando uma string para um método de pesquisa,
o Beautiful Soup irá buscar uma correspondência a esta exata string. O seguinte código
encontrará todas as tags <b> no documento::

 soup.find_all('b')
 # [<b>The Dormouse's story</b>]

Se você passar uma byte string, o Beautiful Soup assumirá que a string
esta codificada como UTF-8. Você pode evitar isso passando ao invés disso
uma string Unicode.

.. _uma expressão regular:

Uma expressão regular (regex)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Se você passar um objeto `regex`, o Beautiful Soup irá
realizar um filtro com ela utilizando seu método ``search()``. 
O código seguinte buscará todas as tags as quais os nomes comecem com
a letra "b"; neste caso, a tag <body> e a tag <b>::

 import re
 for tag in soup.find_all(re.compile("^b")):
     print(tag.name)
 # body
 # b

Este código buscará todas as tags cujo nome contenha a letra "t"::

 for tag in soup.find_all(re.compile("t")):
     print(tag.name)
 # html
 # title

.. _uma lista:

Uma lista
^^^^^^^^^

Se você passar uma lista, o Beautiful Soup irá buscar
uma correspondência com qualquer item dessuma lista.
O código seguinte buscará todas as tags <a> e todas
as tags <b>::

 soup.find_all(["a", "b"])
 # [<b>The Dormouse's story</b>,
 #  <a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>,
 #  <a class="sister" href="http://example.com/lacie" id="link2">Lacie</a>,
 #  <a class="sister" href="http://example.com/tillie" id="link3">Tillie</a>]

.. _the value True:

``True``
^^^^^^^^

O valor ``True`` irá corresponder com tudo.
O código abaixo encontrará ``todas`` as tags do documento,
mas nenhuma das strings::

 for tag in soup.find_all(True):
     print(tag.name)
 # html
 # head
 # title
 # body
 # p
 # b
 # p
 # a
 # a
 # a
 # p

.. _a function:

Uma função
^^^^^^^^^^

Se nenhuma das opções anteriores funcionar para você, defina uma
função que pegará um elemento como seu único argumento. A função
deverá retornar ``True`` se o argumento corresponder e ``False``
caso contrário.

Aqui você tem uma função que irá retornar ``True`` se uma tag definir
o atributo `class`, mas não definir o atributo `id`::

 def has_class_but_no_id(tag):
     return tag.has_attr('class') and not tag.has_attr('id')

Passe esta função dentro de ``find_all()`` e você irá retornar todas
as tags <p>::

 soup.find_all(has_class_but_no_id)
 # [<p class="title"><b>The Dormouse's story</b></p>,
 #  <p class="story">Once upon a time there were...</p>,
 #  <p class="story">...</p>]

Esta função irá encontrar apenas as tags <p>. Não irá encontrar as tags <a>,
porque elas definem "class e "id" ao mesmo tempo. Ela não encontrará
as tags <html> e <title>, porque estas tags não definem um atributo 
"class".

Se você passar uma função para filtrar um atributo específico como
``href``, o argumento passado na função será o nome do atributo e
não toda a tag. Aqui vemos uma função que encontra todas as tags <a>
em que o atributo ``href`` não corresponde a expressão regular passada::

 def not_lacie(href):
     return href and not re.compile("lacie").search(href)
 soup.find_all(href=not_lacie)
 # [<a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>,
 #  <a class="sister" href="http://example.com/tillie" id="link3">Tillie</a>]

A função pode ser tão complexa quanto você precise que seja.
Aqui temos uma função que retorna ``True`` se uma tag esta
cercada por objetos string::

 from bs4 import NavigableString
 def surrounded_by_strings(tag):
     return (isinstance(tag.next_element, NavigableString)
             and isinstance(tag.previous_element, NavigableString))

 for tag in soup.find_all(surrounded_by_strings):
     print tag.name
 # p
 # a
 # a
 # a
 # p

Agora nós estamos prontos para olhar os métodos de busca em detalhes.

``find_all()``
--------------

Definição: find_all(:ref:`name <name>`, :ref:`attrs <attrs>`, :ref:`recursive
<recursive>`, :ref:`string <string>`, :ref:`limit <limit>`, :ref:`**kwargs <kwargs>`)

O método ``find_all()``  busca entre os decendentes de uma tag e retorna todos os decendentes
que correspondem a seus filtros. Dei diversos exemplos em `Tipos de filtros`_,
mas aqui estão mais alguns::

 soup.find_all("title")
 # [<title>The Dormouse's story</title>]

 soup.find_all("p", "title")
 # [<p class="title"><b>The Dormouse's story</b></p>]

 soup.find_all("a")
 # [<a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>,
 #  <a class="sister" href="http://example.com/lacie" id="link2">Lacie</a>,
 #  <a class="sister" href="http://example.com/tillie" id="link3">Tillie</a>]

 soup.find_all(id="link2")
 # [<a class="sister" href="http://example.com/lacie" id="link2">Lacie</a>]

 import re
 soup.find(string=re.compile("sisters"))
 # u'Once upon a time there were three little sisters; and their names were\n'

Alguns podem parecer familiares, mas outros são novos.
O que significa passar um valor ``string`` ou ``id``? Por que
``find_all("p", "title")`` encontra uma tag <p> com a classe CSS "title"?
Vamos dar uma olhada nos argumentos de ``find_all()``.

.. _name:

O argumento ``name``
^^^^^^^^^^^^^^^^^^^^

Passe um valor para ``name`` e você dirá para o Beautiful Soup
considerar apenas as tags com certos nomes. Strings de texto seão ignoradas,
assim como os nomes que não corresponderem ao argumento ``name``

Este é o uso mais simples::

 soup.find_all("title")
 # [<title>The Dormouse's story</title>]

Lembre-se de `Tipos de filtros`_ que o valor para ``name`` pode ser `uma
string`_, `uma expressão regular`_, `uma lista`_, `uma função`_, ou
:ref:`o valor True <the value True>`.

.. _kwargs:

Os argumentos "palavras-chave"
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Qualquer argumento que não for reconhecido se tornará um filtro
de atributos da tag. Se você passar um valor para um argumento
chamado ``id``, o Beautiful Soup irá buscar correspondentes entre 
todas tags ``id``::

 soup.find_all(id='link2')
 # [<a class="sister" href="http://example.com/lacie" id="link2">Lacie</a>]

Se você passar um valor para ``href``, o Beautiful Soup buscar correspondentes
em cada tag que possua o atributo ``href``::

 soup.find_all(href=re.compile("elsie"))
 # [<a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>]

Você pode filtrar um atributo baseado em `uma string`_,
`uma expressão regular`_, `uma lista`_, `uma função`_, ou
:ref:`o valor True <the value True>`.

Este código encontra todas as tags em que o atributo ``id``
possuem um valor, independente de qual valor seja::

 soup.find_all(id=True)
 # [<a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>,
 #  <a class="sister" href="http://example.com/lacie" id="link2">Lacie</a>,
 #  <a class="sister" href="http://example.com/tillie" id="link3">Tillie</a>]

Você pode filtrar múltiplos atributos de uma vez passando mais de um argumento
palavra-chave::

 soup.find_all(href=re.compile("elsie"), id='link1')
 # [<a class="sister" href="http://example.com/elsie" id="link1">three</a>]

Alguns atributos, como o atributo data-* do HTML5, possuem nomes que não
podem ser usados como argumentos palavra-chave:::

 data_soup = BeautifulSoup('<div data-foo="value">foo!</div>')
 data_soup.find_all(data-foo="value")
 # SyntaxError: keyword can't be an expression

Você pode usar estes atributos para realizar buscas, colocando-os
em um dicionário e passando o dicionário em ``find_all()``, como o argumento
``attrs``::

 data_soup.find_all(attrs={"data-foo": "value"})
 # [<div data-foo="value">foo!</div>]

Você não pode utilizar um argumento palavra-chave para buscar pelo elemento
HTML "name", porque o Beautiful Soup utiliza o argumento ``name`` para
conter o nome da própria tag. Ao invés disso, você pode passar o valor para
"name" no argumento ``attrs``::

 name_soup = BeautifulSoup('<input name="email"/>')
 name_soup.find_all(name="email")
 # []
 name_soup.find_all(attrs={"name": "email"})
 # [<input name="email"/>]

.. _attrs:

Buscando por uma classe CSS
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

É muito útil buscar por uma tag que tem uma certa classe CSS, mas
o nome do atributo CSS, "class", é uma palavra reservada no Python.
Utilizar ``class`` como um argumento palavra-chave lhe trará um erro
de sintaxe. A partir do Beautiful Soup 4.1.2, você pode buscar por uma
classe CSS utilizando o argumento palavra-chave ``class_``::

 soup.find_all("a", class_="sister")
 # [<a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>,
 #  <a class="sister" href="http://example.com/lacie" id="link2">Lacie</a>,
 #  <a class="sister" href="http://example.com/tillie" id="link3">Tillie</a>]

Assim como qualquer argumento palavra-chave, você pode passar para ``class_``
uma string, uma expressão regular (regex), uma função ou ``True``::

 soup.find_all(class_=re.compile("itl"))
 # [<p class="title"><b>The Dormouse's story</b></p>]

 def has_six_characters(css_class):
     return css_class is not None and len(css_class) == 6

 soup.find_all(class_=has_six_characters)
 # [<a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>,
 #  <a class="sister" href="http://example.com/lacie" id="link2">Lacie</a>,
 #  <a class="sister" href="http://example.com/tillie" id="link3">Tillie</a>]

:ref:`Lembre-se <multivalue>` que uma tag pode ter valores múltiplos
para seu atributo classe. Quando você buscar por uma tag que tenha
uma certa classe CSS, você esta buscando correspodência em `qualquer`
de suas classes CSS::

 css_soup = BeautifulSoup('<p class="body strikeout"></p>')
 css_soup.find_all("p", class_="strikeout")
 # [<p class="body strikeout"></p>]

 css_soup.find_all("p", class_="body")
 # [<p class="body strikeout"></p>]

Você pode também buscar por uma string exata como valor de ``class``::

 css_soup.find_all("p", class_="body strikeout")
 # [<p class="body strikeout"></p>]

Mas ao procurar por variações de uma string, isso não irá funcionar::

 css_soup.find_all("p", class_="strikeout body")
 # []

Se voce quiser buscar por tags que correspondem a duas ou mais classes CSS, 
você deverá utilizar um seletor CSS::

 css_soup.select("p.strikeout.body")
 # [<p class="body strikeout"></p>]

Em versões mais antigas do Beautiful Soup, as quais não possuem o atalho ``class_``
você pode utilizar o truque ``attrs`` conforme mencionado acima. Será criado um dicionário
do qual o valor para "class" seja uma string ( ou uma expressão regular, ou qualquer
outra coisa) que você queira procurar::

 soup.find_all("a", attrs={"class": "sister"})
 # [<a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>,
 #  <a class="sister" href="http://example.com/lacie" id="link2">Lacie</a>,
 #  <a class="sister" href="http://example.com/tillie" id="link3">Tillie</a>]

.. _string:

O argumento ``string``
^^^^^^^^^^^^^^^^^^^^^^^

Com ``string`` você pode buscar por strings ao invés de tags. Assim como
``name`` e os argumentos palavras-chave, você pode passar `uma string`_, `uma
expressão regular`_, `uma lista`_, `uma função`_, ou
:ref:`o valor True <the value True>`. Aqui estão alguns exemplos::

 soup.find_all(string="Elsie")
 # [u'Elsie']

 soup.find_all(string=["Tillie", "Elsie", "Lacie"])
 # [u'Elsie', u'Lacie', u'Tillie']

 soup.find_all(string=re.compile("Dormouse"))
 [u"The Dormouse's story", u"The Dormouse's story"]

 def is_the_only_string_within_a_tag(s):
     """Return True if this string is the only child of its parent tag."""
     return (s == s.parent.string)

 soup.find_all(string=is_the_only_string_within_a_tag)
 # [u"The Dormouse's story", u"The Dormouse's story", u'Elsie', u'Lacie', u'Tillie', u'...']

Mesmo que ``string`` seja para encontrar strings, você pode combiná-lo com argumentos
para encontrar tags: o Beautiful Soup encontrará todas as tags as quais
``.string`` corresponder seu valor em ``string``. O código seguinte encontra
a tag <a>, a qual a ``.string`` é "Elsie"::

 soup.find_all("a", string="Elsie")
 # [<a href="http://example.com/elsie" class="sister" id="link1">Elsie</a>]

O argumento ``string`` é novo no Beautiful Soup 4.4.0. Em versões anteriores
ele era chamado de ``text``::

 soup.find_all("a", text="Elsie")
 # [<a href="http://example.com/elsie" class="sister" id="link1">Elsie</a>]

.. _limit:

O argumento ``limit``
^^^^^^^^^^^^^^^^^^^^^^

``find_all()`` retorna todas as tags e strings que correspondem aos seus
filtros. Isso pode levar algum tmepo se o documento for extenso. Se você
não precisar de `todos` os resultados, você pode passar um número limite 
(``limit``). Ele funciona assim como o parâmetro LIMIT utilizado em SQL.
Ele diz ao Beautiful Soup para parar de adquirir resultados assim que atingir
um certo número.

Existem três links no documento "three sisters", mas este código encontra somente
os dois primeiros::

 soup.find_all("a", limit=2)
 # [<a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>,
 #  <a class="sister" href="http://example.com/lacie" id="link2">Lacie</a>]

.. _recursive:

O argumento ``recursive``
^^^^^^^^^^^^^^^^^^^^^^^^^^

Se você chamar ``mytag.find_all()``, o Beautiful Soup irá examinar todos os descendentes
de ``mytag``: suas filhas, as filhas de suas filhas e daí em diante. Se você quer apenas que
o Beautiful Soup considere filhas diretas, você pode passar o parâmetro ``recursive=False``.
Veja a diferença aqui::

 soup.html.find_all("title")
 # [<title>The Dormouse's story</title>]

 soup.html.find_all("title", recursive=False)
 # []

Aqui está o trecho do documento::

 <html>
  <head>
   <title>
    The Dormouse's story
   </title>
  </head>
 ...

O tag <title> esta abaixo da tag <html>, mas não está `diretamente`
abaixo de <html>: a tag <head> está no caminho entre elas. O Beautiful Soup encontra a tag
<title> quando é autorizado a olhar todos os descendentes de <html>, mas
quando ``recursive=False`` é restringido o acesso as filhas imediatas de <html>.

O Beautiful Soup oferece diversos métodos de busca na árvore (como vimos acima), e a maioria
deles recebe os mesmos argumentos que ``find_all()``: ``name``,
``attrs``, ``string``, ``limit``, e os argumentos palavras-chave. Mas o
argumento ``recursive`` é diferente: ``find_all()`` e ``find()`` são 
os únicos métodos que o suportam. Passar ``recursive=False`` em um método
como ``find_parents()`` não seria muito útil.

Chamar uma tag é como chamar ``find_all()``
--------------------------------------------

Por ``find_all()`` ser o método mais popular na API de busca do
Beautiful Soup, você pode usar um atalho para ele. Se você tratar
o objeto ``BeautifulSoup`` ou um objeto ``Tag`` como se fosse uma
função, então é o mesmo que chamar ``find_all()`` para aquele objeto.
Estas duas linhas de código são equivalentes::

 soup.find_all("a")
 soup("a")

Estas duas linhas também são equivalentes::

 soup.title.find_all(string=True)
 soup.title(string=True)

``find()``
----------

Signature: find(:ref:`name <name>`, :ref:`attrs <attrs>`, :ref:`recursive
<recursive>`, :ref:`string <string>`, :ref:`**kwargs <kwargs>`)

O método ``find_all()`` varre todo o documento em busca de resultados, 
mas algumas vezes você irá querer apenas um resultado. Se você sabe que
o documento possui apenas uma tag <body>, é perda de tempo varrer todo o
o documento procurando por outras. Ao invés de passar ``limit=1``
toda vez em que chamar ``find_all``, você pode usar o método ``find()``.
Estas duas linhas de código são `quase` equivalentes::

 soup.find_all('title', limit=1)
 # [<title>The Dormouse's story</title>]

 soup.find('title')
 # <title>The Dormouse's story</title>

A única diferença é que ``find_all()`` retorna uma lista contendo apenas
um resuldado, enquanto ``find()`` retorna o resultado.

Se ``find_all()`` não encontrar nada, ele retornará uma lista vazia. Se
``find()`` não encontrar nada, ele retornará ``None``::

 print(soup.find("nosuchtag"))
 # None

Lembre-se do truque ``soup.head.title`` de `Navegar usando os nomes das tags`_?
Aquele truque funciona chamando repetidamente ``find()``::

 soup.head.title
 # <title>The Dormouse's story</title>

 soup.find("head").find("title")
 # <title>The Dormouse's story</title>

``find_parents()`` e ``find_parent()``
----------------------------------------

Signature: find_parents(:ref:`name <name>`, :ref:`attrs <attrs>`, :ref:`string <string>`, :ref:`limit <limit>`, :ref:`**kwargs <kwargs>`)

Signature: find_parent(:ref:`name <name>`, :ref:`attrs <attrs>`, :ref:`string <string>`, :ref:`**kwargs <kwargs>`)

Levei muito tempo cobrindo ``find_all()`` e ``find()`` acima.
O API do Beautiful Soup define dez outros métodos
para buscas na árvore, mas não tenha medo! Cinco destes métodos são
basicamente o mesmo que ``find_all()``, e os outros cinco são basicamente
o mesmo que ``find()``. A única diferença está em qual parte da árvore
eles procuram.

Primeiro vamos considerar ``find_parents()`` e
``find_parent()``. Lembre-se que ``find_all()`` e ``find()`` trabalham
de sua própria maneira descendo através da árvore, procurando pelos
descendentes de uma tag. Estes métodos fazem o contrário: eles trabalham
`subindo` a árvore, procurando pelas `mães` de uma tag (ou string).
Vamos experimentá-los: começando por uma string "enterrada" no documento
"three daughters"::

  a_string = soup.find(string="Lacie")
  a_string
  # u'Lacie'

  a_string.find_parents("a")
  # [<a class="sister" href="http://example.com/lacie" id="link2">Lacie</a>]

  a_string.find_parent("p")
  # <p class="story">Once upon a time there were three little sisters; and their names were
  #  <a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>,
  #  <a class="sister" href="http://example.com/lacie" id="link2">Lacie</a> and
  #  <a class="sister" href="http://example.com/tillie" id="link3">Tillie</a>;
  #  and they lived at the bottom of a well.</p>

  a_string.find_parents("p", class="title")
  # []

Uma das três tags <a> é diretamente um nível superior da string em
questão, então nossa busca a encontra. Uma das três tags <p> é uma mãe
indireta da string e nossa busca também a encontra. Há uma tag <p> com
a classe CSS "title" em algum lugar no documento, mas não é nenhuma das tags mães
da string, portanto, não podemos encontrá-la com ``find_parents()``.

Você já deve ter feito a conexão entre ``find_parent()`` e
``find_parents()``, e os atributos `.parent`_ e `.parents`_ mencionados
anteriormente. A conexão é muito forte. Estes métodos de busca utilizam ``.parents`` 
para iterar sobre todos as mãesS e compara cada um com o filtro passado
para verificar se preenche o requisito.

``find_next_siblings()`` e ``find_next_sibling()``
----------------------------------------------------

Signature: find_next_siblings(:ref:`name <name>`, :ref:`attrs <attrs>`, :ref:`string <string>`, :ref:`limit <limit>`, :ref:`**kwargs <kwargs>`)

Signature: find_next_sibling(:ref:`name <name>`, :ref:`attrs <attrs>`, :ref:`string <string>`, :ref:`**kwargs <kwargs>`)

Estes métodos utilizam :ref:`.next_siblings <sibling-generators>` para
iterar sobre o resto dos filhos de um elemento da árvore. O método
``find_next_siblings()`` retornará todos os filhos que atendem o
requisito ``find_next_sibling()`` retorna apenas o primeiro::

 first_link = soup.a
 first_link
 # <a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>

 first_link.find_next_siblings("a")
 # [<a class="sister" href="http://example.com/lacie" id="link2">Lacie</a>,
 #  <a class="sister" href="http://example.com/tillie" id="link3">Tillie</a>]

 first_story_paragraph = soup.find("p", "story")
 first_story_paragraph.find_next_sibling("p")
 # <p class="story">...</p>

``find_previous_siblings()`` e ``find_previous_sibling()``
------------------------------------------------------------

Signature: find_previous_siblings(:ref:`name <name>`, :ref:`attrs <attrs>`, :ref:`string <string>`, :ref:`limit <limit>`, :ref:`**kwargs <kwargs>`)

Signature: find_previous_sibling(:ref:`name <name>`, :ref:`attrs <attrs>`, :ref:`string <string>`, :ref:`**kwargs <kwargs>`)

Estes métodos utilizam :ref:`.previous_siblings <sibling-generators>` para iterar sobre os filhos de um elemento que
o precede na árvore. O método ``find_previous_siblings()``
retorna todos os filhos que atendem o requisito e
``find_previous_sibling()`` retorna apenas o primeiro::

 last_link = soup.find("a", id="link3")
 last_link
 # <a class="sister" href="http://example.com/tillie" id="link3">Tillie</a>

 last_link.find_previous_siblings("a")
 # [<a class="sister" href="http://example.com/lacie" id="link2">Lacie</a>,
 #  <a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>]

 first_story_paragraph = soup.find("p", "story")
 first_story_paragraph.find_previous_sibling("p")
 # <p class="title"><b>The Dormouse's story</b></p>


``find_all_next()`` e ``find_next()``
---------------------------------------

Signature: find_all_next(:ref:`name <name>`, :ref:`attrs <attrs>`, :ref:`string <string>`, :ref:`limit <limit>`, :ref:`**kwargs <kwargs>`)

Signature: find_next(:ref:`name <name>`, :ref:`attrs <attrs>`, :ref:`string <string>`, :ref:`**kwargs <kwargs>`)

Estes métodos utilizam :ref:`.next_elements <element-generators>` para
iterar sobre qualquer tag e string que aparecer depois da atual no documento.
O método ``find_all_next()`` retorna todos os casos que atendem, e
``find_next()`` retorna somente o primeiro caso::

 first_link = soup.a
 first_link
 # <a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>

 first_link.find_all_next(string=True)
 # [u'Elsie', u',\n', u'Lacie', u' and\n', u'Tillie',
 #  u';\nand they lived at the bottom of a well.', u'\n\n', u'...', u'\n']

 first_link.find_next("p")
 # <p class="story">...</p>

No primeiro exemplo, a string "Elsie" foi encontrada, mesmo estando
dentro da tag <a>. No segundo exemplo, a última tag <p> do documento foi
encontrada, mesmo que não esteja na mesma parte da árvore que <a> onde começamos.
Para estes métodos, o que importa é que um elemento corresponda ao filtro e esteja
depois do elemento de início no documento.

``find_all_previous()`` e ``find_previous()``
-----------------------------------------------

Signature: find_all_previous(:ref:`name <name>`, :ref:`attrs <attrs>`, :ref:`string <string>`, :ref:`limit <limit>`, :ref:`**kwargs <kwargs>`)

Signature: find_previous(:ref:`name <name>`, :ref:`attrs <attrs>`, :ref:`string <string>`, :ref:`**kwargs <kwargs>`)

Estes métodos utilizam :ref:`.previous_elements <element-generators>` para
iterar sobre  as tags e strings que aparecem antes do elemento indicado no argumento.
O método ``find_all_previous()`` retorna todos que correspondem a busca e o método 
``find_previous()`` apenas a primeira correspondência::

 first_link = soup.a
 first_link
 # <a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>

 first_link.find_all_previous("p")
 # [<p class="story">Once upon a time there were three little sisters; ...</p>,
 #  <p class="title"><b>The Dormouse's story</b></p>]

 first_link.find_previous("title")
 # <title>The Dormouse's story</title>

Quando se chama ``find_all_previous("p")`` é encontrado não só o
primeiro parágrafo do documento (o que possui class="title"), mas também o
segundo parágrafo, a tag <p> que contém a tag <a> por onde começamos. 
Isso não deveria ser tão surpreendente: nós estamos olhando para todas as tags
que apareceram anteriormente no documento incluindo aquela onde começamos. Uma
tag <p> que contenha uma tag <a> deve aparecer antes da tag <a> que ela contém.

Seletores CSS
-------------

A partir da versão 4.7.0, o Beautiful Soup suporta a maior parte dos seletores CSS4
através do projeto `SoupSieve <https://facelessuser.github.io/soupsieve/>`_. Se você
instalou o Beautiful Soup através do ``pip``,o SoupSieve foi instalado ao mesmo tempo, 
portanto você não precisará realizar nenhuma etapa adicional.

``BeautifulSoup`` possui um método ``.select()`` o qual utiliza o SoupSieve para
executar um seletor CSS selector sobre um documento a ser analisado e retorna todos os
elementos correspondentes. ``Tag`` possui um método similar que executa um seletor CSS
sobre o conteúdo de uma única tag.

(Versões anteriores do Beautiful Soup também possuem o método ``.select()``,
 mas somente os seletores CSS mais populares são suportados.

A `documentação <https://facelessuser.github.io/soupsieve/>`_ SoupSieve
lista todos os seletores suportados atualmente, mas aqui estão alguns dos
básicos:

Você pode encontrar tags::

 soup.select("title")
 # [<title>The Dormouse's story</title>]

 soup.select("p:nth-of-type(3)")
 # [<p class="story">...</p>]

Encontrar tags aninhadas com outras::
 soup.select("body a")
 # [<a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>,
 #  <a class="sister" href="http://example.com/lacie"  id="link2">Lacie</a>,
 #  <a class="sister" href="http://example.com/tillie" id="link3">Tillie</a>]

 soup.select("html head title")
 # [<title>The Dormouse's story</title>]

Encontrar tags `diretamente` abaixo de outras tags no aninhamento::

 soup.select("head > title")
 # [<title>The Dormouse's story</title>]

 soup.select("p > a")
 # [<a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>,
 #  <a class="sister" href="http://example.com/lacie"  id="link2">Lacie</a>,
 #  <a class="sister" href="http://example.com/tillie" id="link3">Tillie</a>]

 soup.select("p > a:nth-of-type(2)")
 # [<a class="sister" href="http://example.com/lacie" id="link2">Lacie</a>]

 soup.select("p > #link1")
 # [<a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>]

 soup.select("body > a")
 # []

Encontrar as irmãs de alguma tag::

 soup.select("#link1 ~ .sister")
 # [<a class="sister" href="http://example.com/lacie" id="link2">Lacie</a>,
 #  <a class="sister" href="http://example.com/tillie"  id="link3">Tillie</a>]

 soup.select("#link1 + .sister")
 # [<a class="sister" href="http://example.com/lacie" id="link2">Lacie</a>]

Encontrar tags pela classe CSS::

 soup.select(".sister")
 # [<a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>,
 #  <a class="sister" href="http://example.com/lacie" id="link2">Lacie</a>,
 #  <a class="sister" href="http://example.com/tillie" id="link3">Tillie</a>]

 soup.select("[class~=sister]")
 # [<a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>,
 #  <a class="sister" href="http://example.com/lacie" id="link2">Lacie</a>,
 #  <a class="sister" href="http://example.com/tillie" id="link3">Tillie</a>]

Encontrar tags pelo ID::

 soup.select("#link1")
 # [<a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>]

 soup.select("a#link2")
 # [<a class="sister" href="http://example.com/lacie" id="link2">Lacie</a>]

Encontrar tags que se relacionam com qualquer seletor em uma lista de seletores::

 soup.select("#link1,#link2")
 # [<a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>,
 #  <a class="sister" href="http://example.com/lacie" id="link2">Lacie</a>]

Testar a existência de um atributo::

 soup.select('a[href]')
 # [<a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>,
 #  <a class="sister" href="http://example.com/lacie" id="link2">Lacie</a>,
 #  <a class="sister" href="http://example.com/tillie" id="link3">Tillie</a>]

Encontrar tags pelo valor do atributo::

 soup.select('a[href="http://example.com/elsie"]')
 # [<a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>]

 soup.select('a[href^="http://example.com/"]')
 # [<a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>,
 #  <a class="sister" href="http://example.com/lacie" id="link2">Lacie</a>,
 #  <a class="sister" href="http://example.com/tillie" id="link3">Tillie</a>]

 soup.select('a[href$="tillie"]')
 # [<a class="sister" href="http://example.com/tillie" id="link3">Tillie</a>]

 soup.select('a[href*=".com/el"]')
 # [<a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>]

Há outro método chamado ``select_one()``, o qual encontra somente
a primeira tag que combina com um seletor::

 soup.select_one(".sister")
 # <a class="sister" href="http://example.com/elsie" id="link1">Elsie</a>

Se você analisou um XML que define namespaces, você pode 
utilizar nos seletores CSS::

 from bs4 import BeautifulSoup
 xml = """<tag xmlns:ns1="http://namespace1/" xmlns:ns2="http://namespace2/">
  <ns1:child>I'm in namespace 1</ns1:child>
  <ns2:child>I'm in namespace 2</ns2:child>
 </tag> """
 soup = BeautifulSoup(xml, "xml")

 soup.select("child")
 # [<ns1:child>I'm in namespace 1</ns1:child>, <ns2:child>I'm in namespace 2</ns2:child>]

 soup.select("ns1|child", namespaces=namespaces)
 # [<ns1:child>I'm in namespace 1</ns1:child>]

Quando manipulando um seletor CSS que utiliza 
namespaces,o Beautiful Soup utiliza a abreviação do namespace
que encontrou quando estava analisando o documento. Você pode evitar isso
passando um dicionário com suas próprias abreviações::

 namespaces = dict(first="http://namespace1/", second="http://namespace2/")
 soup.select("second|child", namespaces=namespaces)
 # [<ns1:child>I'm in namespace 2</ns1:child>]
 
Todo este negócio de seletor CSS é conveniente
para pessoas que já sabem a sintaxe do seletor CSS.
Você pode fazer tudo isso com a API do BeautifulSoup.
E se os seletores CSS são tudo o que você precisa,
você deveria analisar o documento com lxml: é mais rápido. Mas isso deixa você `combinar`
seletores CSS com a API do Beautiful Soup.

Modificando a árvore
====================

O principal poder do Beautiful Soup está na busca pela árvore, mas você
pode também modificar a árvore e escrever suas modificações como um novo
documento HTML ou XML.

Alterando nomes de tags e atributos
-----------------------------------

Cobri este assunto anteriormente em `Atributos`_, mas vale a pena repetir. Você
pode renomear uma tag, alterar o valor de algum de seus atributos, adicionar novos
atributos e deletar qualquer um deles::

 soup = BeautifulSoup('<b class="boldest">Extremely bold</b>')
 tag = soup.b

 tag.name = "blockquote"
 tag['class'] = 'verybold'
 tag['id'] = 1
 tag
 # <blockquote class="verybold" id="1">Extremely bold</blockquote>

 del tag['class']
 del tag['id']
 tag
 # <blockquote>Extremely bold</blockquote>

Modificando ``.string``
-----------------------

Se você definir o um atributo ``.string`` de uma tag, o conteúdo da
tag será substituido pela string que foi passada::

  markup = '<a href="http://example.com/">I linked to <i>example.com</i></a>'
  soup = BeautifulSoup(markup)

  tag = soup.a
  tag.string = "New link text."
  tag
  # <a href="http://example.com/">New link text.</a>

Cuidado: se a tag conter outra(s) tag(s), ela(s) e todo seu conteúdo
serão destruídos.

``append()``
------------

Você pode adicionar algo no conteúdo de uma tag com ``Tag.append()``. Funciona
da mesma maneira que ``.append()`` de uma lista::

   soup = BeautifulSoup("<a>Foo</a>")
   soup.a.append("Bar")

   soup
   # <html><head></head><body><a>FooBar</a></body></html>
   soup.a.contents
   # [u'Foo', u'Bar']

``extend()``
------------

Com início no Beautiful Soup 4.7.0,  ``Tag`` também suporta um método chamado
``.extend()``, o qual funciona da mesma maneira que chamando ``.extend()`` em
uma lista::

   soup = BeautifulSoup("<a>Soup</a>")
   soup.a.extend(["'s", " ", "on"])

   soup
   # <html><head></head><body><a>Soup's on</a></body></html>
   soup.a.contents
   # [u'Soup', u''s', u' ', u'on']
   
``NavigableString()`` e ``.new_tag()``
-------------------------------------------------

Se você precisar adicionar uma string a um documento, sem problema -- você
pode passar uma string Python através de ``append()``, ou você pode chamar
o construtor ``NavigableString``::

   soup = BeautifulSoup("<b></b>")
   tag = soup.b
   tag.append("Hello")
   new_string = NavigableString(" there")
   tag.append(new_string)
   tag
   # <b>Hello there.</b>
   tag.contents
   # [u'Hello', u' there']

Se você quiser criar um comentário ou alguma outra subclasse de
``NavigableString``, apenas chame o construtor::

   from bs4 import Comment
   new_comment = Comment("Nice to see you.")
   tag.append(new_comment)
   tag
   # <b>Hello there<!--Nice to see you.--></b>
   tag.contents
   # [u'Hello', u' there', u'Nice to see you.']

(Esta é uma funcionalidade nova no Beautiful Soup 4.4.0.)

E se você precisar criar uma nova tag? A melhor solução
é chamar o método ``BeautifulSoup.new_tag()``::

   soup = BeautifulSoup("<b></b>")
   original_tag = soup.b

   new_tag = soup.new_tag("a", href="http://www.example.com")
   original_tag.append(new_tag)
   original_tag
   # <b><a href="http://www.example.com"></a></b>

   new_tag.string = "Link text."
   original_tag
   # <b><a href="http://www.example.com">Link text.</a></b>

Somente o primeiro argumento (o nome da tag) é obrigatório.

``insert()``
------------

``Tag.insert()`` funciona assim como ``Tag.append()``, exceto que o novo elemento
não será inserido ao final do ``.contents`` de sua tag mãe. Ele será inserido em qualquer posição
numérica que você informar. Funciona assim como ``.insert()`` em uma lista::

  markup = '<a href="http://example.com/">I linked to <i>example.com</i></a>'
  soup = BeautifulSoup(markup)
  tag = soup.a

  tag.insert(1, "but did not endorse ")
  tag
  # <a href="http://example.com/">I linked to but did not endorse <i>example.com</i></a>
  tag.contents
  # [u'I linked to ', u'but did not endorse', <i>example.com</i>]

``insert_before()`` e ``insert_after()``
------------------------------------------

O método ``insert_before()`` insere tags ou strings imediatamente antes de algo
na árvore::

   soup = BeautifulSoup("<b>stop</b>")
   tag = soup.new_tag("i")
   tag.string = "Don't"
   soup.b.string.insert_before(tag)
   soup.b
   # <b><i>Don't</i>stop</b>

O método ``insert_after()`` insere tags ou strings imediatamente após algo
na árvore::

   div = soup.new_tag('div')
   div.string = 'ever'
   soup.b.i.insert_after(" you ", div)
   soup.b
   # <b><i>Don't</i> you <div>ever</div> stop</b>
   soup.b.contents
   # [<i>Don't</i>, u' you', <div>ever</div>, u'stop']

``clear()``
-----------

O ``Tag.clear()`` remove o conteúdo de uma tag::

  markup = '<a href="http://example.com/">I linked to <i>example.com</i></a>'
  soup = BeautifulSoup(markup)
  tag = soup.a

  tag.clear()
  tag
  # <a href="http://example.com/"></a>

``extract()``
-------------

O ``PageElement.extract()`` remove uma tag ou string da árvore. Ele retorna
a tag ou string que foi extraída::

  markup = '<a href="http://example.com/">I linked to <i>example.com</i></a>'
  soup = BeautifulSoup(markup)
  a_tag = soup.a

  i_tag = soup.i.extract()

  a_tag
  # <a href="http://example.com/">I linked to</a>

  i_tag
  # <i>example.com</i>

  print(i_tag.parent)
  None

Neste ponto você efetivamente tem duas árvores de análise: uma baseada no objeto
``BeautifulSoup`` que você usou para analisar o documento, e outra baseada na tag que foi
extraída. Você pode também chamar ``extract`` em um filho do elemento que você extraiu::

  my_string = i_tag.string.extract()
  my_string
  # u'example.com'

  print(my_string.parent)
  # None
  i_tag
  # <i></i>


``decompose()``
---------------

O ``Tag.decompose()`` remove uma tag da árvore, então destrói `completamente` ela
e seu conteúdo::

  markup = '<a href="http://example.com/">I linked to <i>example.com</i></a>'
  soup = BeautifulSoup(markup)
  a_tag = soup.a

  soup.i.decompose()

  a_tag
  # <a href="http://example.com/">I linked to</a>


.. _replace_with():

``replace_with()``
------------------

Um ``PageElement.replace_with()`` remove uma tag ou string da árvore e
substitui pela tag ou string que você escolher::

  markup = '<a href="http://example.com/">I linked to <i>example.com</i></a>'
  soup = BeautifulSoup(markup)
  a_tag = soup.a

  new_tag = soup.new_tag("b")
  new_tag.string = "example.net"
  a_tag.i.replace_with(new_tag)

  a_tag
  # <a href="http://example.com/">I linked to <b>example.net</b></a>

``replace_with()`` retorna a tag ou string que foi substituída, então você pode
examiná-la ou adicioná-la novamente em outra parte da árvore.

``wrap()``
----------

O ``PageElement.wrap()`` envelopa um elemento na tag que você especificar. Ele 
retornará o novo empacotador::

 soup = BeautifulSoup("<p>I wish I was bold.</p>")
 soup.p.string.wrap(soup.new_tag("b"))
 # <b>I wish I was bold.</b>

 soup.p.wrap(soup.new_tag("div")
 # <div><p><b>I wish I was bold.</b></p></div>

Este método é novo no Beautiful Soup 4.0.5.

``unwrap()``
---------------------------

O ``Tag.unwrap()`` é o oposto de ``wrap()``. Ele substitui uma tag pelo
que estiver dentro dela. É uma boa maneira de remover marcações::

  markup = '<a href="http://example.com/">I linked to <i>example.com</i></a>'
  soup = BeautifulSoup(markup)
  a_tag = soup.a

  a_tag.i.unwrap()
  a_tag
  # <a href="http://example.com/">I linked to example.com</a>

Assim como ``replace_with()``, ``unwrap()`` retorna a tag que foi
substituída.

``smooth()``
---------------------------

Após chamar vários métodos que modificam a árvore, você pode acabar com um ou dois objetos ``NavigableString`` próximos um ao outro. O Beautiful Soup não tem nenhum problema com isso, mas como isso não pode acontecer em um documento que acabou de ser analisado, você não deve esperar um comportamento como o seguinte::

  soup = BeautifulSoup("<p>A one</p>")
  soup.p.append(", a two")

  soup.p.contents
  # [u'A one', u', a two']

  print(soup.p.encode())
  # <p>A one, a two</p>

  print(soup.p.prettify())
  # <p>
  #  A one
  #  , a two
  # </p>

Você pode chamar ``Tag.smooth()`` para limpar a árvore analisada, consolidando strings adjacentes::

 soup.smooth()

 soup.p.contents
 # [u'A one, a two']

 print(soup.p.prettify())
 # <p>
 #  A one, a two
 # </p>

O método ``smooth()`` é novo no Beautiful Soup 4.8.0.

Saída
======

.. _.prettyprinting:

Pretty-printing
---------------

O método ``prettify()`` irá transformar uma árvore do Beautiful Soup em
uma string Unicode devidamente formatada, com uma linha para cada tag e cada string::

  markup = '<a href="http://example.com/">I linked to <i>example.com</i></a>'
  soup = BeautifulSoup(markup)
  soup.prettify()
  # '<html>\n <head>\n </head>\n <body>\n  <a href="http://example.com/">\n...'

  print(soup.prettify())
  # <html>
  #  <head>
  #  </head>
  #  <body>
  #   <a href="http://example.com/">
  #    I linked to
  #    <i>
  #     example.com
  #    </i>
  #   </a>
  #  </body>
  # </html>

Você pode chamar ``prettify()`` no top-level do objeto ``BeautifulSoup``,
ou em qualquer de seus objetos ``Tag``::

  print(soup.a.prettify())
  # <a href="http://example.com/">
  #  I linked to
  #  <i>
  #   example.com
  #  </i>
  # </a>

Non-pretty printing
-------------------

Se você quer apenas uma string, sem nenhuma formatação, você pode chamar
``unicode()`` ou ``str()`` para o objeto ``BeautifulSoup`` ou uma ``Tag``
dentro dele::

 str(soup)
 # '<html><head></head><body><a href="http://example.com/">I linked to <i>example.com</i></a></body></html>'

 unicode(soup.a)
 # u'<a href="http://example.com/">I linked to <i>example.com</i></a>'

A função ``str()`` retorna uma string codificada em UTF-8. Veja
`Codificação (Encoding)`_ para outras opções.

Você também pode chamar ``encode()`` para ter uma bytestring, e ``decode()``
para ter Unicode.

.. _output_formatters:

Output formatters
-----------------

Se você der para o Beautiful Soup um documento que contém entidades HTML como
"&lquot;", elas serão convertidades em caracteres Unicode::

 soup = BeautifulSoup("&ldquo;Dammit!&rdquo; he said.")
 unicode(soup)
 # u'<html><head></head><body>\u201cDammit!\u201d he said.</body></html>'

Se você converter o documento em uma string, os caracteres Unicode
serão codificados como UTF-8. Você não irá ter suas entidades HTML de volta::

 str(soup)
 # '<html><head></head><body>\xe2\x80\x9cDammit!\xe2\x80\x9d he said.</body></html>'

Por padrão, os únicos caracteres que escapam desta saída são o & e os sinais de <>.
Eles são convertidos em "&amp;", "&lt;",
e "&gt;", com isso o Beautiful Soup não gera HTML e XML inválidos de maneira inadvertida.

 soup = BeautifulSoup("<p>The law firm of Dewey, Cheatem, & Howe</p>")
 soup.p
 # <p>The law firm of Dewey, Cheatem, &amp; Howe</p>

 soup = BeautifulSoup('<a href="http://example.com/?foo=val1&bar=val2">A link</a>')
 soup.a
 # <a href="http://example.com/?foo=val1&amp;bar=val2">A link</a>

Você pode alterar este comportamento informando um valor para o argumento de
``formatter`` para ``prettify()``, ``encode()``, ou
``decode()``. Beautiful Soup reconhece cinco possiveis valores para ``formatter``.

O padrão é ``formatter="minimal"``. Strings sempre serão processadas de maneira a garantir que o Beautiful Soup gere HTML/XML válidos::

 french = "<p>Il a dit &lt;&lt;Sacr&eacute; bleu!&gt;&gt;</p>"
 soup = BeautifulSoup(french)
 print(soup.prettify(formatter="minimal"))
 # <html>
 #  <body>
 #   <p>
 #    Il a dit &lt;&lt;Sacré bleu!&gt;&gt;
 #   </p>
 #  </body>
 # </html>

Se você passar ``formatter="html"``, Beautiful Soup irá converter caracteres
Unicode para entidades HTML sempre que possível::

 print(soup.prettify(formatter="html"))
 # <html>
 #  <body>
 #   <p>
 #    Il a dit &lt;&lt;Sacr&eacute; bleu!&gt;&gt;
 #   </p>
 #  </body>
 # </html>

Se você passar um ``formatter="html5"``, é o mesmo que ``formatter="html"``, 
mas o Beautiful Soup irá omitir a barra de fechamento HTML::

 soup = BeautifulSoup("<br>")
 
 print(soup.encode(formatter="html"))
 # <html><body><br/></body></html>
 
 print(soup.encode(formatter="html5"))
 # <html><body><br></body></html>
 
Se você passar ``formatter=None``, Beautiful Soup não irá modificar
as strings na saída. Esta é a opção mais rápida, mas permitirá que o 
Beautiful Soup gere HTML/XML inválidos, como nestes exemplos::

 print(soup.prettify(formatter=None))
 # <html>
 #  <body>
 #   <p>
 #    Il a dit <<Sacré bleu!>>
 #   </p>
 #  </body>
 # </html>

 link_soup = BeautifulSoup('<a href="http://example.com/?foo=val1&bar=val2">A link</a>')
 print(link_soup.a.encode(formatter=None))
 # <a href="http://example.com/?foo=val1&bar=val2">A link</a>

Se você precisar de controles mais sofisticados sobre sua saída, 
você pode usar a classe ``Formatter`` do Beautiful Soup. Aqui você pode ver um
formatter que converte strings para uppercase, quando elas ocorrem em um nó de texto 
ou em um valor de algum atributo::

 from bs4.formatter import HTMLFormatter
 def uppercase(str):
     return str.upper()
 formatter = HTMLFormatter(uppercase)

 print(soup.prettify(formatter=formatter))
 # <html>
 #  <body>
 #   <p>
 #    IL A DIT <<SACRÉ BLEU!>>
 #   </p>
 #  </body>
 # </html>

 print(link_soup.a.prettify(formatter=formatter))
 # <a href="HTTP://EXAMPLE.COM/?FOO=VAL1&BAR=VAL2">
 #  A LINK
 # </a>

Dividindo em subclasses ``HTMLFormatter`` ou ``XMLFormatter`` darão a você ainda
mais controle sobre a saída. Por exemplo, o Beautiful Soup ordena os atributos em toda
tag por padrão::

 attr_soup = BeautifulSoup(b'<p z="1" m="2" a="3"></p>')
 print(attr_soup.p.encode())
 # <p a="3" m="2" z="1"></p>

Para desabilitar esta opção, você pode criar uma subclasse do método ``Formatter.attributes()``,
o qual controla qual atributo será usado na saída e em que ordem. Esta
implementação também filtra o atributido chamado "m" quando ele aparece::

 class UnsortedAttributes(HTMLFormatter):
     def attributes(self, tag):
         for k, v in tag.attrs.items():
             if k == 'm':
	         continue
             yield k, v
 print(attr_soup.p.encode(formatter=UnsortedAttributes())) 
 # <p z="1" a="3"></p>

Um último conselho: se você criar um objeto ``CDATA``, o texto dentro deste objeto
sempre estará presente `exatamente como aparenta, com nenhuma formatação`.
O Beautiful Soup irá chamar sua função de substituição da entidade, apenas
no caso de você ter escrito uma função personalizada que conta todas as strings
que existem no documento ou algo do tipo, mas ele irá ignorar o valor de retorno::

 from bs4.element import CData
 soup = BeautifulSoup("<a></a>")
 soup.a.string = CData("one < three")
 print(soup.a.prettify(formatter="xml"))
 # <a>
 #  <![CDATA[one < three]]>
 # </a>


``get_text()``
--------------

Se você quer apenas o texto contido no documento ou em um par de tags, você
pode utilizar o método ``get_text()``. Ele retornará todo texto em um documento
ou dentro das tags como uma string Unicode::

  markup = '<a href="http://example.com/">\nI linked to <i>example.com</i>\n</a>'
  soup = BeautifulSoup(markup)

  soup.get_text()
  u'\nI linked to example.com\n'
  soup.i.get_text()
  u'example.com'

Você pode especificar uma string a ser usada para unir as partes do texto::

 # soup.get_text("|")
 u'\nI linked to |example.com|\n'

Você pode dizer ao Beautiful Soup para excluir espaços em branco do início
e fim de cada parte de texto::

 # soup.get_text("|", strip=True)
 u'I linked to|example.com'

Contudo para isso, você pode querer utilizar o gerador :ref:`.stripped_strings <string-generators>`
e processar o texto você mesmo::

 [text for text in soup.stripped_strings]
 # [u'I linked to', u'example.com']

Especificando um interpretador (parser) para uso
================================================

Se você precisa analisar um pequeno HTML, você pode passá-lo no construtor do
``BeautifulSoup`` e será o suficiente. O Beautiful Soup irá escolher um parser 
para você e irá interpretar o dado. Mas existem alguns argumentos adicionais que você
pode passar no construtor para alterar qual parser será usado.

O primeiro argumento do construtor ``BeautifulSoup`` é uma string ou uma variável contendo o
conteúdo do que você quer analisar. O segundo argumento é `como` você quer interpretar aquele
conteúdo.

Se você não especificar nada, você irá utilizar o melhor analisador HTML instalado.
O Beautiful Soup classifica o lxml's como sendo o melhor, logo em seguida o html5lib, 
e então o parser nativo do Python. Você pode substituí-lo, especificando de acordo
com as seguintes características:

* O tipo de marcação que você quer analisar. Atualmente são suportados
  "html", "xml", and "html5".
* O nome do parser que você quer utilizar. Atualmente são suportadas
  as opções "lxml", "html5lib", e "html.parser" (parser nativo do Python).

A seção `Instalando um interpretador (parser)` compara os parsers suportados.

Se você não tem um parser apropriado instalado, o Beautiful Soup irá
ignorar sua solicitação e escolher um diferente. Atualmente, o único parser
XML suportado é o lxml. Se você não possui o lxml instalado, pedir um parser
XML não trará um e pedir por "lxml" não funcionará também.


.. _differences-between-parsers:

Diferenças entre os interpretadores (parsers)
---------------------------------------------

O Beautiful Soup apresenta a mesma interface para diferentes parsers,
mas cada um é diferente. Diferentes parsers irão criar diferentes análises da árvore
do mesmo documento. As maiores diferenças estão entre os parsers HTML e XML.
Aqui está um pequeno documento analisado como HTML::

 BeautifulSoup("<a><b /></a>")
 # <html><head></head><body><a><b></b></a></body></html>

Como uma tag <b /> vazia não é um HTML válido, o analisador a transforma
em um par <b></b>.

Aqui está o mesmo documento analisado como XML (partindo do princípio
que você tenha o lxml instalado). Note que o a tag vazia <b /> é deixada sozinha,
e que é dada ao documento uma declaração XML ao invés de ser colocada dentro de uma tag <html>.::

 BeautifulSoup("<a><b /></a>", "xml")
 # <?xml version="1.0" encoding="utf-8"?>
 # <a><b/></a>

Há também diferenças entre analisadores HTML. Se você der ao Beautiful
Soup um documento HTML perfeitamente formatado, estas diferenças não irão
importar. Um analisador será mais rápido que outro, mas todos irão lhe
retornar uma estrutura de dados que se parece exatamente como o HTML original.

Mas se o documento não estiver perfeitamente formatado, diferentes analisadores
irão retornar diferentes resultados. Aqui está um pequeno e inválido documento
analisado utilizando o analisador lxml HTML. Note que a tag pendente </p> é 
simplesmente ignorada::

 BeautifulSoup("<a></p>", "lxml")
 # <html><body><a></a></body></html>

Aqui está o mesmo documento analisado utilizando html5lib::

 BeautifulSoup("<a></p>", "html5lib")
 # <html><head></head><body><a><p></p></a></body></html>

Ao invés de ignorar a tag </p> pendente, o html5lib a equipara a uma tag
<p> aberta. Este parser também adiciona uma tag <head> vazia ao documento.

Aqui está o mesmo documento analisado com o parser HTML nativo do Python::

 BeautifulSoup("<a></p>", "html.parser")
 # <a></a>

Assim como html5lib, este parser ignora a tag de fechamento </p>.
Este parser também não realiza nenhuma tentatida de criar um HTML bem
formatado adicionando uma tag <body>. Como lxml, ele nem se importa em
adicionar uma tag <html>.

Sendo o documento "<a></p>" inválido, nenhuma dessas técnicas é a maneira
"correta" de lidar com isso. O html5lib utiliza técnicas que são parte
do padrão HTML5, portanto vendo sendo definido como a maneira "mais correta", 
mas todas as três técnicas são legítimas.

Diferenças entre analisadores podem afetar o seu script. Se você está
planejando distribuir seu script para outras pessoas, ou rodá-lo em 
múltiplas máquinas, você deve especificar o analisador no construtor
``BeautifulSoup``. Isso irá reduzir as chances de que seus usuários
analisem um documento de forma diferente da maneira como você analisou.

   
Codificação (Encoding)
======================

Todo documento HTML ou XML é escrito em uma codificação (encoding) específica como ASCII
ou UTF-8. Mas quando você carrega um documento no BeautifulSoup, você irá descobrir
que ele foi convertido para Unicode::

 markup = "<h1>Sacr\xc3\xa9 bleu!</h1>"
 soup = BeautifulSoup(markup)
 soup.h1
 # <h1>Sacré bleu!</h1>
 soup.h1.string
 # u'Sacr\xe9 bleu!'

Não é mágica (Seria bem legal que fosse). O BeautifulSoup utiliza uma
sub-biblioteca chamada `Unicode, Dammit`_ para detectar a codificação de
um documento e convertê-lo para Unicode. A codificação detectada automaticamente está 
disponível como objeto ``.original_encoding`` atributo do objeto ``BeautifulSoup`` ::

 soup.original_encoding
 'utf-8'

`Unicode, Dammit` acerta na maioria das vezes, mas pode errar em algumas.
Outras vezes acerta, porém somente após uma busca byte a byte no documento,
o leva muito tempo. Se você souber com antecedência a codificação, você poderá
evitar erros ou demora passando-o para o contrutor do ``BeautifulSoup``
através de ``from_encoding``.

Abaixo você tem um documento escrito em ISO-8859-8. O documento é tão
pequeno que o `Unicode, Dammit` não consegue verificar sua codificação 
e acaba fazendo a identificação como ISO-8859-7::

 markup = b"<h1>\xed\xe5\xec\xf9</h1>"
 soup = BeautifulSoup(markup)
 soup.h1
 <h1>νεμω</h1>
 soup.original_encoding
 'ISO-8859-7'

Podemos consertar isso passando a codificação correta com ``from_encoding``::

 soup = BeautifulSoup(markup, from_encoding="iso-8859-8")
 soup.h1
 <h1>םולש</h1>
 soup.original_encoding
 'iso8859-8'

Se você não sabe qual a codificação correta, mas você sabe que o
`Unicode, Dammit` está errado, você pode passar as opções excluentes 
como ``exclude_encodings``::

 soup = BeautifulSoup(markup, exclude_encodings=["ISO-8859-7"])
 soup.h1
 <h1>םולש</h1>
 soup.original_encoding
 'WINDOWS-1255'

Windows-1255 não é 100% correto, mas é um superconjunto compatível com
ISO-8859-8, portanto é mais próximo do ideal. (``exclude_encodings``
é uma opção nova no Beautiful Soup 4.4.0.)

Em casos raros (geralmente quando um documento UTF-8 contém texto escrito
em uma codificação completamente diferente), a única maneira de ser convertido para
Unicode é convertendo alguns caracteres com o caractere especial Unicode 
"REPLACEMENT CHARACTER" (U+FFFD, �). Se o `Unicode, Dammit` precisar utilizá-lo,
ele será armazenado no atributo ``.contains_replacement_characters`` como 
``True`` no ``UnicodeDammit`` ou objeto ``BeautifulSoup``. Isso deixa você ciente
que a representação Unicode não é uma representação exata do original - algum dado
foi perdido. Se um documento possui �, mas ``.contains_replacement_characters`` é ``False``,
você poderá concluir então que o � já estava ali originalmente e não representa dados
perdidos.

Codificação de Saída
--------------------

Quando um documento é gerado pelo Beautiful Soup, ele é gerado como UTF-8,
mesmo que o documento não for um UTF-8 de início. Aqui está um documento gerado
com codificação Latin-1::

 markup = b'''
  <html>
   <head>
    <meta content="text/html; charset=ISO-Latin-1" http-equiv="Content-type" />
   </head>
   <body>
    <p>Sacr\xe9 bleu!</p>
   </body>
  </html>
 '''

 soup = BeautifulSoup(markup)
 print(soup.prettify())
 # <html>
 #  <head>
 #   <meta content="text/html; charset=utf-8" http-equiv="Content-type" />
 #  </head>
 #  <body>
 #   <p>
 #    Sacré bleu!
 #   </p>
 #  </body>
 # </html>

Note que a tag <meta> foi reescrita para refletir o fato que o documento
é agora um UTF-8.

Se você não quiser um UTF-8, você pode passar a codificação desejada como parâmetro de
``prettify()``::

 print(soup.prettify("latin-1"))
 # <html>
 #  <head>
 #   <meta content="text/html; charset=latin-1" http-equiv="Content-type" />
 # ...

Você também pode chamar encode() no objeto ``BeautifulSoup``  ou em qualquer elemento
do objeto, assim como se faz em uma string Python::

 soup.p.encode("latin-1")
 # '<p>Sacr\xe9 bleu!</p>'

 soup.p.encode("utf-8")
 # '<p>Sacr\xc3\xa9 bleu!</p>'

Qualquer caractere que não pode ser representado na codificação escolhida
irá ser convertida para uma entidade de referência numérica XML. Abaixo você
tem um documento que inclui o caractere Unicode SNOWMAN::

 markup = u"<b>\N{SNOWMAN}</b>"
 snowman_soup = BeautifulSoup(markup)
 tag = snowman_soup.b

O caractere SNOWMAN faz parte da documentação UTF-8 (algo como
☃), mas não possui representação para este caractere em ISO-latin-1 ou
ASCII, portanto ele é convertido para "&#9731" para as essas codificações::

 print(tag.encode("utf-8"))
 # <b>☃</b>

 print tag.encode("latin-1")
 # <b>&#9731;</b>

 print tag.encode("ascii")
 # <b>&#9731;</b>

Unicode, Dammit
---------------

Você pode usar o `Unicode, Dammit` fora do Beautiful Soup. É útil
quando você possui dados em uma codificação desconhecida e quer
simplesmente convertê-la para Unicode::

 from bs4 import UnicodeDammit
 dammit = UnicodeDammit("Sacr\xc3\xa9 bleu!")
 print(dammit.unicode_markup)
 # Sacré bleu!
 dammit.original_encoding
 # 'utf-8'


As respostas do `Unicode, Dammit` serão um pouco mais precisas se você
instalar as bibliotecas ``chardet`` ou ``cchardet``. Quanto maior a quantidade
de dados no arquivo que você passar para o `Unicode, Dammit`, mais precisas serão
as conversões. Se você possui suas suspeitas sobre qual a codificação original,
você pode passar as opções em uma lista::

 dammit = UnicodeDammit("Sacr\xe9 bleu!", ["latin-1", "iso-8859-1"])
 print(dammit.unicode_markup)
 # Sacré bleu!
 dammit.original_encoding
 # 'latin-1'

`Unicode, Dammit` possui duas características que o Beautiful Soup não utiliza.

Smart quotes
^^^^^^^^^^^^

Você pode utilizar `Unicode, Dammit` para converter Microsoft smart quotes para 
entidades HTML ou XML::

 markup = b"<p>I just \x93love\x94 Microsoft Word\x92s smart quotes</p>"

 UnicodeDammit(markup, ["windows-1252"], smart_quotes_to="html").unicode_markup
 # u'<p>I just &ldquo;love&rdquo; Microsoft Word&rsquo;s smart quotes</p>'

 UnicodeDammit(markup, ["windows-1252"], smart_quotes_to="xml").unicode_markup
 # u'<p>I just &#x201C;love&#x201D; Microsoft Word&#x2019;s smart quotes</p>'

Você também pode converter Microsoft smart quotes para ASCII::

 UnicodeDammit(markup, ["windows-1252"], smart_quotes_to="ascii").unicode_markup
 # u'<p>I just "love" Microsoft Word\'s smart quotes</p>'

Espero que você ache estas características úteis, mas o Beautiful Soup não
as usa.O Beautiful Soup dá preferência ao comportamento padrão, que é
converter para caracteres Unicode::

 UnicodeDammit(markup, ["windows-1252"]).unicode_markup
 # u'<p>I just \u201clove\u201d Microsoft Word\u2019s smart quotes</p>'

Codificação Inconsistente
^^^^^^^^^^^^^^^^^^^^^^^^^

Algumas vezes um documento é em sua maioria UTF-8, mas contém  caracteres 
Windows-1252 assim como (de novo) Microsoft smart quotes. Isso pode acontecer
quando um website compostos de dados de muitas fontes diferentes. Você pode
utilizar ``UnicodeDammit.detwingle()`` para transformar este documento em um
UTF-8 puro. Aqui está um exemplo::

 snowmen = (u"\N{SNOWMAN}" * 3)
 quote = (u"\N{LEFT DOUBLE QUOTATION MARK}I like snowmen!\N{RIGHT DOUBLE QUOTATION MARK}")
 doc = snowmen.encode("utf8") + quote.encode("windows_1252")

Este documento é uma bagunça. O snowmen é um UTF-8 e as aspas são Windows-1252. 
Você pode exibir o snowmen ou as aspas, mas não os dois ao mesmo tempo::

 print(doc)
 # ☃☃☃�I like snowmen!�

 print(doc.decode("windows-1252"))
 # â˜ƒâ˜ƒâ˜ƒ“I like snowmen!”

Decodificar um documento como UTF-8 gera um ``UnicodeDecodeError``, e
como um Windows-1252 lhe tras algo sem sentido. Felizmente,
``UnicodeDammit.detwingle()`` irá converter a string para UTF-8 puro,
permitindo a você decodificá-la para Unicode e exibir o snowmen e as
aspas simultaneamente::

 new_doc = UnicodeDammit.detwingle(doc)
 print(new_doc.decode("utf8"))
 # ☃☃☃“I like snowmen!”

``UnicodeDammit.detwingle()`` sabe apenas como trabalhar com Windows-1252
contido em UTF-8 (ou vice versa, eu suponho), mas este é o caso mais comum.

Note que você deve chamar ``UnicodeDammit.detwingle()`` em seu dado
antes de passá-lo para ``BeautifulSoup`` ou para o construtor ``UnicodeDammit``. 
O Beautiful Soup assume que um documento possui apenas uma codificação,
independente de qual ela seja. Se você passar um documento que
contém ambos UTF-8 e Windows-1252, é provável que ele pense que todo
o documento seja Windows-1252, e o documento parecerá ``â˜ƒâ˜ƒâ˜ƒ“I like snowmen!”``.

``UnicodeDammit.detwingle()`` é novo no Beautiful Soup 4.1.0.

Linhas numeradas
================

Os interpretadores ``html.parser` e ``html5lib`` podem rastrear onde, no
documento original, cada tag foi encontrada. Você pode acessar esta
informação através de ``Tag.sourceline`` (número da linha) e ``Tag.sourcepos``
(posição do início da tag na linha)::

   markup = "<p\n>Paragraph 1</p>\n    <p>Paragraph 2</p>"
   soup = BeautifulSoup(markup, 'html.parser')
   for tag in soup.find_all('p'):
       print(tag.sourceline, tag.sourcepos, tag.string)
   # (1, 0, u'Paragraph 1')
   # (2, 3, u'Paragraph 2')

Note que os dois interpretadores significam coisas levemente diferentes por
``sourceline`` e ``sourcepos``. Para html.parser, estes números representam 
a posição do sinal `menor que`inicial. Para html5lib, representa a posição 
do sinal `maior que` final::
   
   soup = BeautifulSoup(markup, 'html5lib')
   for tag in soup.find_all('p'):
       print(tag.sourceline, tag.sourcepos, tag.string)
   # (2, 1, u'Paragraph 1')
   # (3, 7, u'Paragraph 2')

Você pode desabilitar esta característica passando ``store_line_numbers=False`
no construtor ``BeautifulSoup``::

   markup = "<p\n>Paragraph 1</p>\n    <p>Paragraph 2</p>"
   soup = BeautifulSoup(markup, 'html.parser', store_line_numbers=False)
   soup.p.sourceline
   # None
  
Esta característica é nova no 4.8.1 e os analisadores baseados no lxml
não a suportam.

Comparando objetos por igualdade
================================

O Beautiful Soup diz que dois objetos ``NavigableString`` ou ``Tag`` são
iguais quando eles apresentam as mesma marcação HTML ou XML. No exemplo
abaixo, as duas tags <b> são tratadas como iguais, mesmo estando em partes
diferentes da árvore do objeto, porque ambas estão como "<b>pizza</b>"::

 markup = "<p>I want <b>pizza</b> and more <b>pizza</b>!</p>"
 soup = BeautifulSoup(markup, 'html.parser')
 first_b, second_b = soup.find_all('b')
 print first_b == second_b
 # True

 print first_b.previous_element == second_b.previous_element
 # False

Se você quiser verificar se duas variáveis se referem exatamente ao
mesmo objeto, use `is`::

 print first_b is second_b
 # False

Copiando objetos Beautiful Soup
===============================

Você pode utilizar ``copy.copy()`` para criar uma cópia de qualquer ``Tag`` ou
``NavigableString``::

 import copy
 p_copy = copy.copy(soup.p)
 print p_copy
 # <p>I want <b>pizza</b> and more <b>pizza</b>!</p>


A cópia será considerada igual ao original, desde que ela apresente a mesma
marcação que o original, mas não será o mesmo objeto::

 print soup.p == p_copy
 # True

 print soup.p is p_copy
 # False

A única diferença real é que a cópia é completamente separada da árvore
original do Beautiful Soup, como se ``extract()`` fosse chamado para ela::

 print p_copy.parent
 # None

Isso acontece porque dois objetos ``Tag`` diferentes não podem ocupar o mesmo
espaço ao mesmo tempo.


Analisando apenas parte de um documento
=======================================

Suponhamos que você queira que o Beautiful Soup olhe apenas para as
tags <a> de um documento. É um desperdício de tempo e memória analisar
todo o documento e, posteriormente, analisar novamente apenas para buscar
as tags <a>. Seria muito mais rápido ignorar tudo o que não for <a> em
primeiro lugar. A classe ``SoupStrainer`` permite que você escolha
qual partes do documento serão analisadas. Você deverá penas criar uma
instância de ``SoupStrainer`` e passá-la ao construtor ``BeautifulSoup`` 
no argumento ``parse_only``.

(Note que *esta característica não funcionará se você estiver utilizando
o html5lib*. Se você utilizar o html5lib, todo o documento será analisado.
Isso acontece porque html5lib constantemente reorganiza a árvore de análise
e se alguma parte do documento realmente não fizer parte dela, ela irá quebrar.
Para evitar confusão, no exemplo abaixo, forçarei o Beautiful Soup a usar o
analisador nativo do Python).

``SoupStrainer``
----------------

A classe ``SoupStrainer`` recebe os mesmos argumentos que qualquer método em `Buscando na árvore`_: :ref:`name <name>`, :ref:`attrs
<attrs>`, :ref:`string <string>`, e :ref:`**kwargs <kwargs>`. Aqui temos três objetos ``SoupStrainer`` ::

 from bs4 import SoupStrainer

 only_a_tags = SoupStrainer("a")

 only_tags_with_id_link2 = SoupStrainer(id="link2")

 def is_short_string(string):
     return len(string) < 10

 only_short_strings = SoupStrainer(string=is_short_string)

Irei trazer de volta o documento "three sisters" mais uma vez e veremos
como o documento se parece quando é analisado com estes três objetos ``SoupStrainer``
diferentes::

 html_doc = """
 <html><head><title>The Dormouse's story</title></head>
 <body>
 <p class="title"><b>The Dormouse's story</b></p>

 <p class="story">Once upon a time there were three little sisters; and their names were
 <a href="http://example.com/elsie" class="sister" id="link1">Elsie</a>,
 <a href="http://example.com/lacie" class="sister" id="link2">Lacie</a> and
 <a href="http://example.com/tillie" class="sister" id="link3">Tillie</a>;
 and they lived at the bottom of a well.</p>

 <p class="story">...</p>
 """

 print(BeautifulSoup(html_doc, "html.parser", parse_only=only_a_tags).prettify())
 # <a class="sister" href="http://example.com/elsie" id="link1">
 #  Elsie
 # </a>
 # <a class="sister" href="http://example.com/lacie" id="link2">
 #  Lacie
 # </a>
 # <a class="sister" href="http://example.com/tillie" id="link3">
 #  Tillie
 # </a>

 print(BeautifulSoup(html_doc, "html.parser", parse_only=only_tags_with_id_link2).prettify())
 # <a class="sister" href="http://example.com/lacie" id="link2">
 #  Lacie
 # </a>

 print(BeautifulSoup(html_doc, "html.parser", parse_only=only_short_strings).prettify())
 # Elsie
 # ,
 # Lacie
 # and
 # Tillie
 # ...
 #

Você pode também passar um ``SoupStrainer`` em qualquer método coberto em `Buscando na árvore`_. 
Este uso provavelmente não seja muito útil, mas pensei que deveria mencioná-lo::

 soup = BeautifulSoup(html_doc)
 soup.find_all(only_short_strings)
 # [u'\n\n', u'\n\n', u'Elsie', u',\n', u'Lacie', u' and\n', u'Tillie',
 #  u'\n\n', u'...', u'\n']

Solucionando Problemas
======================

.. _diagnose:

``diagnose()``
--------------

Se você está tendo problemas em entender o que o Beautiful Soup está
fazendo com um documento, passe o documento pela função ``diagnose()``. (Nova no Beautiful Soup 4.2.0.)
O Beautiful Soup irá retornar um relatório mostrando como diferentes parsers
lidam com o documento e irá lhe dizer o Beautiful Soup poderia estar utilizando outro parser::

 from bs4.diagnose import diagnose
 with open("bad.html") as fp:
     data = fp.read()
 diagnose(data)

 # Diagnostic running on Beautiful Soup 4.2.0
 # Python version 2.7.3 (default, Aug  1 2012, 05:16:07)
 # I noticed that html5lib is not installed. Installing it may help.
 # Found lxml version 2.3.2.0
 #
 # Trying to parse your data with html.parser
 # Here's what html.parser did with the document:
 # ...

Olhando para o que diagnose() retorna, poderá lhe dizer como resolver
o seu problema. Mesmo que não consiga, você poderá colar a saída de ``diagnose()``
quando solicitar ajuda.

Erros enquanto se analisa um documento
--------------------------------------

Existem dois tipos diferentes de erros de análise. Existem quebras
quando você passa para o Beautiful Soup um documento e ele retorna uma 
exceção, geralmente um ``HTMLParser.HTMLParseError``. E existe o comportamento
inesperado, quando uma árvore de análise parece um pouco diferente do
documento usado para criá-la.

Quase nenhum destes problemas são parte do Beautiful Soup. Não é
porque o Beautiful Soup é maravilhosamente um software bem escrito. É
porque o Beautiful Soup não inclui nenhum código de análise. Ao invés disso,
ele depende de analisadores externos. Se um analisador não funciona com
certo documento, a melhor solução é tentar um analisador diferente. Veja 
:ref:`Instalando um interpretador <parser-installation>` para detalhes e uma comparação entre eles.

Os erros de interpretação mais comuns são ``HTMLParser.HTMLParseError:
malformed start tag`` e ``HTMLParser.HTMLParseError: bad end
tag``. Existem dois parsers gerados para o parser built in do Python
e a solução é  :ref:`install lxml ou html5lib. <parser-installation>`

Os tipos de erros de comportamento inesperado mais comuns acontecem
quando não é encontrada a tag buscada no documento. Você vê a busca
sendo executada, mas ``find_all()`` retorna ``[]`` ou ``find()`` retorna ``None``. 
Este é um problema comum com o analisador HTML nativo do Python que algumas
vezes pula tags que ele não entende. Novamente, a solução é
:ref:`instalar o lxml ou html5lib.<parser-installation>`

Problemas de incompatibilidade de versões
-----------------------------------------

* ``SyntaxError: Invalid syntax`` (on the line ``ROOT_TAG_NAME =
  u'[document]'``): Causado por rodar a versão Python 2 do
  Beautiful Soup no Python 3, sem converter o código.

* ``ImportError: No module named HTMLParser`` - Causado por rodar a
  versão Python 2  do Beautiful Soup no Python 3.

* ``ImportError: No module named html.parser`` - Causado por rodar a
  versão Python 3 do Beautiful Soup no Python 2.

* ``ImportError: No module named BeautifulSoup`` - Causado por rodar
  código do Beautiful Soup 3 em um sistema que não possui o BS3
  instalado. Ou por escrever código Beautiful Soup 4 sem saber que
  o nome do pacote é diferente no ``bs4``.

* ``ImportError: No module named bs4`` - Causado por rodar código Beautiful
  Soup 4 em um sistema que não possui o BS4 instalado.

.. _parsing-xml:

Analisando um XML
-----------------

Por padrão, o Beautiful Soup analisa documento como HTML. Para analisar um documento
como XML, passe "xml" como um segundo argumento ao construtor ``BeautifulSoup`` ::

 soup = BeautifulSoup(markup, "xml")

Você precisará ter :ref:` lxml instalado <parser-installation>`.

Outros problemas com analisadores
---------------------------------

* Se seu script funciona em um computador, mas não em outro,
  ou em um ambiente virtual mas não em outro, ou fora do ambiente
  virtual mas não dentro dele, provavelmente porque ambos os ambientes
  possuem bibliotecas de analisadores difererentes. Por exemplo, você pode
  ter desenvolvido um script em um computador que possui lxml instalado,
  e então estar tentando rodá-lo no seu computador que possui apenas html5lib
  instalado. Veja :ref:`Diferenças entre os interpretadores (parsers) <differences-between-parsers>` para entender porque isso importa,
  e corrija o problema mencionando uma biblioteca específica no construtor ``BeautifulSoup``.

* Por tags `HTML e atributos serem case-insensitive
  <http://www.w3.org/TR/html5/syntax.html#syntax>`_, todos os três
  parsers HTML convertem tags e atributos para lowercase. Isso é,
  a marcação <TAG></TAG> é convertida para <tag></tag>. Se você quiser
  preservar a formatação anterior das tags e atributos, você precisará
  :ref:`analisar o documento como XML. <parsing-xml>`

.. _misc:

Diversos
--------

* ``UnicodeEncodeError: 'charmap' codec can't encode character
  u'\xfoo' in position bar`` (ou qualquer outro
  ``UnicodeEncodeError``) - Este não é um problema do Beautiful Soup.
  Este problema poderá surgir em duas situações: a primeira quando você
  tentar imprimir um caractere Unicode que seu console não sabe como 
  exibir. (Veja `Esta página na wiki do Python
  <http://wiki.python.org/moin/PrintFails>`_ para saber mais.). A segunda,
  quando você está gravando um arquivo e passa um caractere Unicode que
  não é suportado pelo seu codificador padrão. Neste caso, a solução mais
  simples é explicitamente converter a string Unicode em UTF-8 com
  ``u.encode("utf8")``.

* ``KeyError: [attr]`` - Caused by accessing ``tag['attr']`` quando a
  tag em questão não define o atributo ``attr``. Os erros mais comuns são 
  ``KeyError: 'href'`` e ``KeyError:
  'class'``. Use ``tag.get('attr')`` se você não tem certeza se ``attr`` está
  definido, assim como você faria em um dicionário Python.

* ``AttributeError: 'ResultSet' object has no attribute 'foo'`` - Isso
  geralmente ocorre quando você espera que ``find_all()`` retorne
  uma única tag ou string. Mas ``find_all()`` retorn uma _lista_ de tags
  e strings--um objeto ``ResultSet``. Você precisa iterar sobre a lista e
  buscar ``.foo`` para cada um. Ou, se você realmente quiser apenas um resultado,
  deverá usar ``find()`` ao invés de ``find_all()``.

* ``AttributeError: 'NoneType' object has no attribute 'foo'`` - Isso
  geralmente acontece quando é chamado ``find()`` e então se tenta acessar
  o atributo `.foo`` o resultado. Mas no seu caso, ``find()`` não encontra nada,
  então retorna ``None`` ao invés de retornar uma tag ou uma string. Você precisa
  descobrir porque ``find()`` não está retornando nada.

Melhorando a performance
------------------------

O Beautiful Soup nunca será tão rápido quanto os parsers em que
ele foi construido em cima. Se o tempo de resposta se tornar crítico,
se você estiver pagando por hora de uso de um computador ou se há 
qualquer outra razão para que o tempo de processamento seja mais
valioso que o tempo de programação, você deve esquecer o Beautiful Soup 
e trabalhar diretamente em cima do `lxml <http://lxml.de/>`_.

Dito isso, existem algumas coisas que você pode fazer para acelerar o 
Beautiful Soup. Se você não está utilizando o lxml como seu parser,
meu conselho é que o faça :ref:`start <parser-installation>`. 
O Beautiful Soup analisa documentos significativamente mais rápido
utilizando o lxml do que usando o html.parser ou html5lib.

Você pode acelerar a detecção da codificação significativamente instalando
a biblioteca `cchardet <http://pypi.python.org/pypi/cchardet/>`_ .

`Analisando apenas parte de um documento`_ não irá lhe poupar muito tempo de
análise, mas irá poupar muita memória e fará a `busca` no documento muito
mais rápida.

Beautiful Soup 3
================

O Beautiful Soup 3 é a versão anterior e não é mais desenvolvida
ativamente. Ela atualmente faz parte da maioria das distribuições
Linux:

:kbd:`$ apt-get install python-beautifulsoup`

Também está publicada no PyPi como ``BeautifulSoup``.:

:kbd:`$ easy_install BeautifulSoup`

:kbd:`$ pip install BeautifulSoup`

Você também pode fazer o `download de um tarball do Beautiful Soup 3.2.0
<http://www.crummy.com/software/BeautifulSoup/bs3/download/3.x/BeautifulSoup-3.2.0.tar.gz>`_.

Se você rodar ``easy_install beautifulsoup`` ou ``easy_install
BeautifulSoup``, mas seu código não funcionar, você instalou o Beautiful
Soup 3 por engano. Você precisa executar ``easy_install beautifulsoup4``.

`A documentação do Beautiful Soup 3 está arquivada online
<http://www.crummy.com/software/BeautifulSoup/bs3/documentation.html>`_.

Portabilidade de código para BS4
--------------------------------

A maioria dos códigos escritos em Beautiful Soup 3 irá funcionar no 
Beautiful Soup 4 com uma pequena alteração. Tudo que você precisa
fazer é alterar o nome do pacote de ``BeautifulSoup`` para ``bs4``. Então::

  from BeautifulSoup import BeautifulSoup

deverá ser assim::

  from bs4 import BeautifulSoup

* Se for gerado um ``ImportError`` "No module named BeautifulSoup", o
  problema é que você está tentando executar um código Beautiful Soup 3,
  mas possui apenas o Beautiful Soup 4 instalado.

* Se for gerado um ``ImportError`` "No module named bs4", o problema
  é que você está tentando executar um código Beautiful Soup 4, mas
  possui apenas o Beautiful Soup 3 instalado.

Apesar do BS4 ser quase totalmente compativel com BS3, a maioria de seus 
métodos foram depreciados e renomeados para atender o padrão `PEP 8
<http://www.python.org/dev/peps/pep-0008/>`_. Existem muitas outras
renomeações e alterações, e algumas delas quebram esta compatibilidade.

Aqui está o que você irá precisar saber para converter seu código BS3 para BS4:

Você precisa de um interpretador (parser)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

O Beautiful Soup 3 utilizava o ``SGMLParser`` do Python, um módulo que
foi depreciado e removido no Python 3.0. O Beautiful Soup 4 utiliza o
``html.parser`` por padrão, mas você pode adicionar o lxml ou html5lib 
e utilizá-los como alternativa. Veja :ref:`Instalando um interpretador <parser-installation>` para 
comparação.

Como o ``html.parser`` não é o mesmo analisador que ``SGMLParser``, é possível
que o Beautiful Soup 4 retorne uma árvore de análise diferente da 
gerada pelo Beautiful Soup 3 para as mesmas marcações. Se você trocar 
``html.parser`` por lxml ou html5lib, você poderá descorbrir que a árvore também
mudará. Se isso acontecer, você precisará atualizar seu código para lidar com a
nova árvore.

Nomes dos Métodos
^^^^^^^^^^^^^^^^^

* ``renderContents`` -> ``encode_contents``
* ``replaceWith`` -> ``replace_with``
* ``replaceWithChildren`` -> ``unwrap``
* ``findAll`` -> ``find_all``
* ``findAllNext`` -> ``find_all_next``
* ``findAllPrevious`` -> ``find_all_previous``
* ``findNext`` -> ``find_next``
* ``findNextSibling`` -> ``find_next_sibling``
* ``findNextSiblings`` -> ``find_next_siblings``
* ``findParent`` -> ``find_parent``
* ``findParents`` -> ``find_parents``
* ``findPrevious`` -> ``find_previous``
* ``findPreviousSibling`` -> ``find_previous_sibling``
* ``findPreviousSiblings`` -> ``find_previous_siblings``
* ``getText`` -> ``get_text``
* ``nextSibling`` -> ``next_sibling``
* ``previousSibling`` -> ``previous_sibling``

Alguns argumentos do construtor do Beautiful Soup foram renomeados pelas
mesmas razões:

* ``BeautifulSoup(parseOnlyThese=...)`` -> ``BeautifulSoup(parse_only=...)``
* ``BeautifulSoup(fromEncoding=...)`` -> ``BeautifulSoup(from_encoding=...)``

Eu renomeei um método para compatibilidade com Python 3:

* ``Tag.has_key()`` -> ``Tag.has_attr()``

Eu renomeei um atributo para utilizar uma terminologia mais precisa:

* ``Tag.isSelfClosing`` -> ``Tag.is_empty_element``

Eu renomeei três atributos para evitar utilizar palavras reservadas do 
Python. Ao contrário das outras, estas alterações *não são compativeis com
versões anteriores.* Se você utilizar estes atributos no BS3, seu código 
irá quebrar no BS4 até você corrigí-los.

* ``UnicodeDammit.unicode`` -> ``UnicodeDammit.unicode_markup``
* ``Tag.next`` -> ``Tag.next_element``
* ``Tag.previous`` -> ``Tag.previous_element``

Geradores
^^^^^^^^^

Eu dei nomes aos geradores de acordo com o PEP-8 e transformei-os
em propriedades:

* ``childGenerator()`` -> ``children``
* ``nextGenerator()`` -> ``next_elements``
* ``nextSiblingGenerator()`` -> ``next_siblings``
* ``previousGenerator()`` -> ``previous_elements``
* ``previousSiblingGenerator()`` -> ``previous_siblings``
* ``recursiveChildGenerator()`` -> ``descendants``
* ``parentGenerator()`` -> ``parents``

Então, ao invés de::

 for parent in tag.parentGenerator():
     ...

Você pode escrever::

 for parent in tag.parents:
     ...

(Mas a versão antiga ainda funcionará.)

Alguns dos geradores eram utilizados para gerar ``None`` após 
finalizado e então parar. Isso era um bug. Agora os geradores
apenas param.

Existem dois novos geradores, :ref:`.strings e
.stripped_strings <string-generators>`. ``.strings`` gera objetos
NavigableString, e ``.stripped_strings`` gera strings Python com 
espaços em branco removidos.

XML
^^^
Não existe mais uma classe ``BeautifulStoneSoup`` para analisar XML. Para
analisar XML você deverá passar "xml" como segundo argumento ao construtor 
``BeautifulSoup``. Pela mesma razão, o construtor ``BeautifulSoup`` não
reconhece mais o argumento ``isHTML``.

A manipulação do Beautiful Soup's de tags XML vazias foi melhorada. 
Anteriormente, quando você analisava um XML, deveria explicitamente
dizer quais tags seriam consideradas elementos de tag vazios. O 
argumento ``selfClosingTags`` não é mais reconhecido. Ao invés disso, 
o Beautiful Soup considera qualquer tag vazia como um elemento de tag vazia. 
Se você adicionar uma filha a um elemento de tag vazia, ela deixará de ser vazia.

Entidades
^^^^^^^^^

Uma entidade HTML ou XML de entrada é sempre convertida em
seu caractere Unicode correspondente. O Beautiful Soup 3 possuia
inúmeras maneiras redundantes de lidar com entidades, as quais foram
removidas. O construtor ``BeautifulSoup`` não reconhece mais os argumentos
``smartQuotesTo`` ou ``convertEntities``. (`Unicode,
Dammit`_ ainda possui ``smart_quotes_to``, mas seu padrão agora é converter 
smart quotes em Unicode.) As constantes ``HTML_ENTITIES``,
``XML_ENTITIES``, e ``XHTML_ENTITIES``  foram removidas, desde que elas
se referiam a uma feature (transformar algumas, mas não todas as entidades
em caracteres Unicode) que não existe mais.
Se você quiser transformar caracteres Unicode novamente em entidades HTML
na saída, ao invés de transformá-las em caracteres UTF-8, você precisará
utilizar um :ref:`output formatter <output_formatters>`.

Variados
^^^^^^^^

:ref:`Tag.string <.string>` agora opera recursivamente. Se a tag A
contém apenas uma tag B e nada mais, então A.string é o mesmo que
B.string. (Anteriormente era None)

`Atributos com múltiplos valores`_ como ``class`` possuem listas de strings
como valores e não strings. Isso deverá afetar a maneira que você buscará
por classes CSS.

Se você passar um dos métodos ``find*``, ambos :ref:`string <string>` `e`
um argumento específico de uma tag como :ref:`name <name>`, o Beautiful Soup
irá buscar por tags que atentem o seu critério de argumento específico e que
:ref:`Tag.string <.string>` atenda o valor para :ref:`string <string>`. Isso 
`não` irá encontrar as strings por si. Anteriormente, Beautiful Soup ignorava
o argumento específico  de uma tag e olhava apenas para as strings.

O construtor ``BeautifulSoup`` não reconhece mais o argumento `markupMassage`. 
É agora responsabilidade do parser de manipular a marcação corretamente.

As classes raramente usadas do analisador como
``ICantBelieveItsBeautifulSoup`` e ``BeautifulSOAP`` foram removidas. 
é agora decisão do analisador como manipular marcações ambiguas.

O método ``prettify()`` agora retorna uma string Unicode, e não bytestring.
