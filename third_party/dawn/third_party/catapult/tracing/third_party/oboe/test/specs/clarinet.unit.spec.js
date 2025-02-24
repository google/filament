
// test cases borrowed from
//    https://github.com/dscape/clarinet/blob/master/test/clarinet.js
var docs   =
    {
      empty_input:
      { text      : '',
        events    :
        [ // bit of a hack that on completely empty
          // input an empty object is parsed out -
          // in future this should be simply zero
          // events
          [SAX_VALUE_OPEN, {}],
          [SAX_VALUE_CLOSE]
        ]
      }
    , empty_array :
      { text      : '[]'
      , events    :
        [ [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_CLOSE , undefined]
        ]
      }
    , just_slash :
      { text      : '["\\\\"]'
      , events    :
        [ [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, "\\"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]


        ]
      }
    , zero_byte    :
      { text       : '{"foo": "\\u0000"}'
      , events     :
        [ [SAX_VALUE_OPEN, {}]
        , [SAX_KEY         , "foo"]
        , [SAX_VALUE_OPEN, "\u0000"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]


        ]
      }
    , empty_value  :
      { text       : '{"foo": ""}'
      , events     :
        [ [SAX_VALUE_OPEN, {}]
        , [SAX_KEY         , "foo"]
        , [SAX_VALUE_OPEN, ""], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]


        ]
      }
    , three_byte_utf8 :
      { text          : '{"matzue": "松江", "asakusa": "浅草"}'
      , events        :
        [ [SAX_VALUE_OPEN, {}]
        , [SAX_KEY         , "matzue"]
        , [SAX_VALUE_OPEN, "松江"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "asakusa"]
        , [SAX_VALUE_OPEN, "浅草"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]


        ]
      }
    , four_byte_utf8 :
      { text          : '{ "U+10ABCD": "􊯍" }'
      , events        :
        [ [SAX_VALUE_OPEN, {}]
        , [SAX_KEY         , "U+10ABCD"]
        , [SAX_VALUE_OPEN, "􊯍"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]


        ]
      }
    , bulgarian    :
      { text       : '["Да Му Еба Майката"]'
      , events     :
        [ [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, "Да Му Еба Майката"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE  , undefined]


        ]
      }
    , codepoints_from_unicodes  :
      { text       : '["\\u004d\\u0430\\u4e8c\\ud800\\udf02"]'
      , events     :
        [ [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, "\u004d\u0430\u4e8c\ud800\udf02"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE  , undefined]


        ]
      }
    , empty_object :
      { text       : '{}'
      , events     :
        [ [SAX_VALUE_OPEN, {}]
        , [SAX_VALUE_CLOSE , undefined]


        ]
      }
    , foobar   :
      { text   : '{"foo": "bar"}'
      , events :
        [ [SAX_VALUE_OPEN, {}]
        , [SAX_KEY         , "foo"]
        , [SAX_VALUE_OPEN, "bar"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]


        ]
      }
    , as_is    :
      { text   : "{\"foo\": \"its \\\"as is\\\", \\\"yeah\", \"bar\": false}"
      , events :
        [ [SAX_VALUE_OPEN, {}]
        , [SAX_KEY         , "foo"]
        , [SAX_VALUE_OPEN, 'its "as is", "yeah'], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "bar"]
        , [SAX_VALUE_OPEN, false], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]


        ]
      }
    , array    :
      { text   : '["one", "two"]'
      , events :
        [ [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, 'one'], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, 'two'], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]


        ]
      }
    , array_fu :
      { text   : '["foo", "bar", "baz",true,false,null,{"key":"value"},' +
                 '[null,null,null,[]]," \\\\ "]'
      , events :
        [ [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, 'foo'], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, 'bar'], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, 'baz'], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, true], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, false], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, null], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, {}]
        , [SAX_KEY          , 'key']
        , [SAX_VALUE_OPEN, 'value'], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]
        , [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, null], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, null], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, null], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_CLOSE  , undefined]
        , [SAX_VALUE_CLOSE  , undefined]
        , [SAX_VALUE_OPEN, " \\ "], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE  , undefined]


        ]
      }
    , simple_exp    :
      { text   : '[10e-01]'
      , events :
        [ [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, 10e-01], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]


        ]
      }
    , nested   :
      { text   : '{"a":{"b":"c"}}'
      , events :
        [ [SAX_VALUE_OPEN, {}]
        , [SAX_KEY         , "a"]
        , [SAX_VALUE_OPEN, {}]
        , [SAX_KEY         , "b"]
        , [SAX_VALUE_OPEN, "c"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]
        , [SAX_VALUE_CLOSE , undefined]


        ]
      }
    , nested_array  :
      { text        : '{"a":["b", "c"]}'
      , events      :
          [ [SAX_VALUE_OPEN, {}]
          , [SAX_KEY         , "a"]
          , [SAX_VALUE_OPEN, []]
          , [SAX_VALUE_OPEN, 'b'], [SAX_VALUE_CLOSE, undefined]
          , [SAX_VALUE_OPEN, 'c'], [SAX_VALUE_CLOSE, undefined]
          , [SAX_VALUE_CLOSE  , undefined]
          , [SAX_VALUE_CLOSE , undefined]


          ]
      }
    , array_of_objs :
      { text        : '[{"a":"b"}, {"c":"d"}]'
      , events      :
        [ [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, {}]
        , [SAX_KEY          , 'a']
        , [SAX_VALUE_OPEN, 'b'], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]
        , [SAX_VALUE_OPEN, {}]
        , [SAX_KEY          , 'c']
        , [SAX_VALUE_OPEN, 'd'], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]
        , [SAX_VALUE_CLOSE  , undefined]


        ]
      }
    , two_keys  :
      { text    : '{"a": "b", "c": "d"}'
      , events  :
        [ [SAX_VALUE_OPEN, {}]
        , [SAX_KEY         , "a"]
        , [SAX_VALUE_OPEN, "b"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "c"]
        , [SAX_VALUE_OPEN, "d"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]


        ]
      }
    , key_true  :
      { text    : '{"foo": true, "bar": false, "baz": null}'
      , events  :
        [ [SAX_VALUE_OPEN, {}]
        , [SAX_KEY         , "foo"]
        , [SAX_VALUE_OPEN, true], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "bar"]
        , [SAX_VALUE_OPEN, false], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "baz"]
        , [SAX_VALUE_OPEN, null], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]


        ]
      }
    , obj_strange_strings  :
      { text               :
        '{"foo": "bar and all\\\"", "bar": "its \\\"nice\\\""}'
      , events             :
        [ [SAX_VALUE_OPEN, {}]
        , [SAX_KEY           , "foo"]
        , [SAX_VALUE_OPEN, 'bar and all"'], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY           , "bar"]
        , [SAX_VALUE_OPEN, 'its "nice"'], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE   , undefined]


        ]
      }
    , bad_foo_bar         :
      { text              :
          '["foo", "bar"'
       , events           :
         [ [SAX_VALUE_OPEN, []]
         , [SAX_VALUE_OPEN, 'foo'], [SAX_VALUE_CLOSE, undefined]
         , [SAX_VALUE_OPEN, 'bar'], [SAX_VALUE_CLOSE, undefined]
         , [FAIL_EVENT       , undefined]
         ]
       }
    , string_invalid_escape:
      { text             :
          '["and you can\'t escape thi\s"]'
       , events          :
         [ [SAX_VALUE_OPEN, []]
         , [SAX_VALUE_OPEN, 'and you can\'t escape this'], [SAX_VALUE_CLOSE, undefined]
         , [SAX_VALUE_CLOSE  , undefined]
        ]
       }
    , nuts_and_bolts :
      { text         : '{"boolean, true": true' +
                       ', "boolean, false": false' +
                       ', "null": null }'
       , events          :
         [ [SAX_VALUE_OPEN, {}]
         , [SAX_KEY          , "boolean, true"]
         , [SAX_VALUE_OPEN, true], [SAX_VALUE_CLOSE, undefined]
         , [SAX_KEY          , "boolean, false"]
         , [SAX_VALUE_OPEN, false], [SAX_VALUE_CLOSE, undefined]
         , [SAX_KEY          , "null"]
         , [SAX_VALUE_OPEN, null], [SAX_VALUE_CLOSE, undefined]
         , [SAX_VALUE_CLOSE  , undefined]


         ]
      }
    , frekin_string:
      { text    : '["\\\\\\"\\"a\\""]'
      , events  :
        [ [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, '\\\"\"a\"'], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE  , undefined]


        ]
      }
    , array_of_string_insanity  :
      { text    : '["\\\"and this string has an escape at the beginning",' +
                  '"and this string has no escapes"]'
      , events  :
        [ [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, "\"and this string has an escape at the beginning"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, "and this string has no escapes"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE  , undefined]


        ]
      }
    , non_utf8           :
      { text   :
        '{"CoreletAPIVersion":2,"CoreletType":"standalone",' +
        '"documentation":"A corelet that provides the capability to upload' +
        ' a folder’s contents into a user’s locker.","functions":[' +
        '{"documentation":"Displays a dialog box that allows user to ' +
        'select a folder on the local system.","name":' +
        '"ShowBrowseDialog","parameters":[{"documentation":"The ' +
        'callback function for results.","name":"callback","required":' +
        'true,"type":"callback"}]},{"documentation":"Uploads all mp3 files' +
        ' in the folder provided.","name":"UploadFolder","parameters":' +
        '[{"documentation":"The path to upload mp3 files from."' +
        ',"name":"path","required":true,"type":"string"},{"documentation":' +
        ' "The callback function for progress.","name":"callback",' +
        '"required":true,"type":"callback"}]},{"documentation":"Returns' +
        ' the server name to the current locker service.",' +
        '"name":"GetLockerService","parameters":[]},{"documentation":' +
        '"Changes the name of the locker service.","name":"SetLockerSer' +
        'vice","parameters":[{"documentation":"The value of the locker' +
        ' service to set active.","name":"LockerService","required":true' +
        ',"type":"string"}]},{"documentation":"Downloads locker files to' +
        ' the suggested folder.","name":"DownloadFile","parameters":[{"' +
        'documentation":"The origin path of the locker file.",' +
        '"name":"path","required":true,"type":"string"},{"documentation"' +
        ':"The Window destination path of the locker file.",' +
        '"name":"destination","required":true,"type":"integer"},{"docum' +
        'entation":"The callback function for progress.","name":' +
        '"callback","required":true,"type":"callback"}]}],' +
        '"name":"LockerUploader","version":{"major":0,' +
        '"micro":1,"minor":0},"versionString":"0.0.1"}'
      , events :
        [ [SAX_VALUE_OPEN, {}], [SAX_KEY, "CoreletAPIVersion" ]
        , [SAX_VALUE_OPEN, 2 ], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "CoreletType"]
        , [SAX_VALUE_OPEN, "standalone" ], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "documentation"]
        , [SAX_VALUE_OPEN, "A corelet that provides the capability to upload a folder’s contents into a user’s locker."], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY          , "functions"]
        , [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, {}], [SAX_KEY, "documentation" ]
        , [SAX_VALUE_OPEN, "Displays a dialog box that allows user to select a folder on the local system."], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "name"]
        , [SAX_VALUE_OPEN, "ShowBrowseDialog"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "parameters"]
        , [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, {}], [SAX_KEY, "documentation" ]
        , [SAX_VALUE_OPEN, "The callback function for results."], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "name"]
        , [SAX_VALUE_OPEN, "callback"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "required"]
        , [SAX_VALUE_OPEN, true], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "type"]
        , [SAX_VALUE_OPEN, "callback"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]
        , [SAX_VALUE_CLOSE  , undefined]
        , [SAX_VALUE_CLOSE , undefined]
        , [SAX_VALUE_OPEN, {}], [SAX_KEY, "documentation" ]
        , [SAX_VALUE_OPEN, "Uploads all mp3 files in the folder provided."], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "name"]
        , [SAX_VALUE_OPEN, "UploadFolder"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "parameters"]
        , [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, {}], [SAX_KEY, "documentation" ]
        , [SAX_VALUE_OPEN, "The path to upload mp3 files from."], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "name"]
        , [SAX_VALUE_OPEN, "path"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "required"]
        , [SAX_VALUE_OPEN, true], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "type"]
        , [SAX_VALUE_OPEN, "string"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]
        , [SAX_VALUE_OPEN, {}], [SAX_KEY, "documentation" ]
        , [SAX_VALUE_OPEN, "The callback function for progress."], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "name"]
        , [SAX_VALUE_OPEN, "callback"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "required"]
        , [SAX_VALUE_OPEN, true ], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "type"]
        , [SAX_VALUE_OPEN, "callback"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]
        , [SAX_VALUE_CLOSE  , undefined]
        , [SAX_VALUE_CLOSE , undefined]
        , [SAX_VALUE_OPEN, {}], [SAX_KEY, "documentation" ]
        , [SAX_VALUE_OPEN, "Returns the server name to the current locker service."], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "name"]
        , [SAX_VALUE_OPEN, "GetLockerService"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "parameters"]
        , [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_CLOSE  , undefined]
        , [SAX_VALUE_CLOSE , undefined]
        , [SAX_VALUE_OPEN, {}], [SAX_KEY, "documentation" ]
        , [SAX_VALUE_OPEN, "Changes the name of the locker service."], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "name"]
        , [SAX_VALUE_OPEN, "SetLockerService"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "parameters"]
        , [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, {}], [SAX_KEY, "documentation" ]
        , [SAX_VALUE_OPEN, "The value of the locker service to set active."], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "name"]
        , [SAX_VALUE_OPEN, "LockerService" ], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "required" ]
        , [SAX_VALUE_OPEN, true], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "type"]
        , [SAX_VALUE_OPEN, "string"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]
        , [SAX_VALUE_CLOSE  , undefined]
        , [SAX_VALUE_CLOSE , undefined]
        , [SAX_VALUE_OPEN, {}], [SAX_KEY, "documentation" ]
        , [SAX_VALUE_OPEN, "Downloads locker files to the suggested folder."], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "name"]
        , [SAX_VALUE_OPEN, "DownloadFile"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "parameters"]
        , [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, {}], [SAX_KEY, "documentation" ]
        , [SAX_VALUE_OPEN, "The origin path of the locker file."], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "name"]
        , [SAX_VALUE_OPEN, "path"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "required"]
        , [SAX_VALUE_OPEN, true], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "type"]
        , [SAX_VALUE_OPEN, "string"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]
        , [SAX_VALUE_OPEN, {}], [SAX_KEY, "documentation" ]
        , [SAX_VALUE_OPEN, "The Window destination path of the locker file."], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "name"]
        , [SAX_VALUE_OPEN, "destination"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "required"]
        , [SAX_VALUE_OPEN, true], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "type"]
        , [SAX_VALUE_OPEN, "integer"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]
        , [SAX_VALUE_OPEN, {}], [SAX_KEY, "documentation" ]
        , [SAX_VALUE_OPEN, "The callback function for progress."], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "name"]
        , [SAX_VALUE_OPEN, "callback"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "required"]
        , [SAX_VALUE_OPEN, true], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "type"]
        , [SAX_VALUE_OPEN, "callback"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]
        , [SAX_VALUE_CLOSE  , undefined]
        , [SAX_VALUE_CLOSE , undefined]
        , [SAX_VALUE_CLOSE  , undefined]
        , [SAX_KEY         , "name"]
        , [SAX_VALUE_OPEN, "LockerUploader"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "version"]
        , [SAX_VALUE_OPEN, {}], [SAX_KEY, "major" ]
        , [SAX_VALUE_OPEN, 0], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "micro"]
        , [SAX_VALUE_OPEN, 1], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "minor"]
        , [SAX_VALUE_OPEN, 0], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]
        , [SAX_KEY         , "versionString"]
        , [SAX_VALUE_OPEN, "0.0.1"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]
        ]
      }
    , array_of_arrays    :
      { text   : '[[[["foo"]]]]'
      , events :
        [ [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, "foo"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]
        , [SAX_VALUE_CLOSE , undefined]
        , [SAX_VALUE_CLOSE , undefined]
        , [SAX_VALUE_CLOSE , undefined]


        ]
      }
    , low_overflow :
      { text       : '[-9223372036854775808]'
      , events     :
        [ [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, -9223372036854775808], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]


        ]
      }
    , high_overflow :
      { text       : '[9223372036854775808]'
      , events     :
        [ [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, 9223372036854775808], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]


        ]
      }
    , floats       :
      { text       : '[0.1e2, 1e1, 3.141569, 10000000000000e-10]'
      , events     :
        [ [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, 0.1e2], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, 1e1], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, 3.141569], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, 10000000000000e-10], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]


        ]
      }
    , numbers_game :
      { text       : '[1,0,-1,-0.3,0.3,1343.32,3345,3.1e124,'+
                     ' 9223372036854775807,-9223372036854775807,0.1e2, ' +
                     '1e1, 3.141569, 10000000000000e-10,' +
                     '0.00011999999999999999, 6E-06, 6E-06, 1E-06, 1E-06,'+
                     '"2009-10-20@20:38:21.539575", 9223372036854775808,' +
                     '123456789,-123456789,' +
                     '2147483647, -2147483647]'
      , events     :
        [ [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, 1], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, 0], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, -1], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, -0.3], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, 0.3], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, 1343.32], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, 3345], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, 3.1e124], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, 9223372036854775807], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, -9223372036854775807], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, 0.1e2], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, 1e1], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, 3.141569], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, 10000000000000e-10], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, 0.00011999999999999999], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, 6E-06], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, 6E-06], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, 1E-06], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, 1E-06], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, "2009-10-20@20:38:21.539575"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, 9223372036854775808], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, 123456789], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, -123456789], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, 2147483647], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, -2147483647], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]


        ]
      }
    , johnsmith  :
      { text     : '{ "firstName": "John", "lastName" : "Smith", "age" : ' +
                   '25, "address" : { "streetAddress": "21 2nd Street", ' +
                   '"city" : "New York", "state" : "NY", "postalCode" : ' +
                   ' "10021" }, "phoneNumber": [ { "type" : "home", ' +
                   '"number": "212 555-1234" }, { "type" : "fax", ' +
                   '"number": "646 555-4567" } ] }'
      , events   :
        [ [SAX_VALUE_OPEN, {}]
        , [SAX_KEY          , "firstName"]
        , [SAX_VALUE_OPEN, "John"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY          , "lastName"]
        , [SAX_VALUE_OPEN, "Smith"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY          , "age"]
        , [SAX_VALUE_OPEN, 25], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY          , "address"]
        , [SAX_VALUE_OPEN, {}]
        , [SAX_KEY          , "streetAddress"]
        , [SAX_VALUE_OPEN, "21 2nd Street"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY          , "city"]
        , [SAX_VALUE_OPEN, "New York"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY          , "state"]
        , [SAX_VALUE_OPEN, "NY"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY          , "postalCode"]
        , [SAX_VALUE_OPEN, "10021"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]
        , [SAX_KEY          , "phoneNumber"]
        , [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, {}]
        , [SAX_KEY          , "type"]
        , [SAX_VALUE_OPEN, "home"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY          , "number"]
        , [SAX_VALUE_OPEN, "212 555-1234"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]
        , [SAX_VALUE_OPEN, {}]
        , [SAX_KEY          , "type"]
        , [SAX_VALUE_OPEN, "fax"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY          , "number"]
        , [SAX_VALUE_OPEN, "646 555-4567"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE  , undefined]
        , [SAX_VALUE_CLOSE   , undefined]
        , [SAX_VALUE_CLOSE  , undefined]
        ]
      }
    , array_null :
      { text     : '[null,false,true]'
      , events   :
        [ [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, null], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, false], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, true], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE  , undefined]
        ]
      }
    , empty_array_comma :
      { text    : '{"a":[],"c": {}, "b": true}'
      , events  :
        [ [SAX_VALUE_OPEN, {}]
        , [SAX_KEY          , "a"]
        , [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_CLOSE  , undefined]
        , [SAX_KEY         , "c"]
        , [SAX_VALUE_OPEN, {}]
        , [SAX_VALUE_CLOSE , undefined]
        , [SAX_KEY         , "b"]
        , [SAX_VALUE_OPEN, true], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]


        ]
      }
    , incomplete_json_terminates_ending_in_number :
      { text    : '[[1,2,3],[4,5'
      , events  :
        [ [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, 1], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, 2], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, 3], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE  , undefined]
        , [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, 4], [SAX_VALUE_CLOSE, undefined]
        , [FAIL_EVENT       , undefined]
        ]
      }
    , incomplete_json_terminates_ending_in_comma :
      { text    : '[[1,2,3],'
      , events  :
        [ [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, 1], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, 2], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, 3], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE  , undefined]
        , [FAIL_EVENT       , undefined]
        ]
      }
    , json_org  :
      { text    :
          ('{\r\n' +
          '          "glossary": {\n' +
          '              "title": "example glossary",\n\r' +
          '      \t\t"GlossDiv": {\r\n' +
          '                  "title": "S",\r\n' +
          '      \t\t\t"GlossList": {\r\n' +
          '                      "GlossEntry": {\r\n' +
          '                          "ID": "SGML",\r\n' +
          '      \t\t\t\t\t"SortAs": "SGML",\r\n' +
          '      \t\t\t\t\t"GlossTerm": "Standard Generalized ' +
          'Markup Language",\r\n' +
          '      \t\t\t\t\t"Acronym": "SGML",\r\n' +
          '      \t\t\t\t\t"Abbrev": "ISO 8879:1986",\r\n' +
          '      \t\t\t\t\t"GlossDef": {\r\n' +
          '                              "para": "A meta-markup language,' +
          ' used to create markup languages such as DocBook.",\r\n' +
          '      \t\t\t\t\t\t"GlossSeeAlso": ["GML", "XML"]\r\n' +
          '                          },\r\n' +
          '      \t\t\t\t\t"GlossSee": "markup"\r\n' +
          '                      }\r\n' +
          '                  }\r\n' +
          '              }\r\n' +
          '          }\r\n' +
          '      }\r\n')
      , events  :
        [ [SAX_VALUE_OPEN, {}]
        , [SAX_KEY         , "glossary"]
        , [SAX_VALUE_OPEN, {}]
        , [SAX_KEY         , "title"]
        , [SAX_VALUE_OPEN, "example glossary"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "GlossDiv"]
        , [SAX_VALUE_OPEN, {}]
        , [SAX_KEY         , "title"]
        , [SAX_VALUE_OPEN, "S"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "GlossList"]
        , [SAX_VALUE_OPEN, {}]
        , [SAX_KEY         , "GlossEntry"]
        , [SAX_VALUE_OPEN, {}]
        , [SAX_KEY         , "ID"]
        , [SAX_VALUE_OPEN, "SGML"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "SortAs"]
        , [SAX_VALUE_OPEN, "SGML"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "GlossTerm"]
        , [SAX_VALUE_OPEN, "Standard Generalized Markup Language"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "Acronym"]
        , [SAX_VALUE_OPEN, "SGML"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "Abbrev"]
        , [SAX_VALUE_OPEN, 'ISO 8879:1986'], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "GlossDef"]
        , [SAX_VALUE_OPEN, {}]
        , [SAX_KEY         , "para"]
        , [SAX_VALUE_OPEN, 'A meta-markup language, used to create markup languages such as DocBook.'], [SAX_VALUE_CLOSE, undefined]
        , [SAX_KEY         , "GlossSeeAlso"]
        , [SAX_VALUE_OPEN, []]
        , [SAX_VALUE_OPEN, "GML"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_OPEN, "XML"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE  , undefined]
        , [SAX_VALUE_CLOSE , undefined]
        , [SAX_KEY         , "GlossSee"]
        , [SAX_VALUE_OPEN, "markup"], [SAX_VALUE_CLOSE, undefined]
        , [SAX_VALUE_CLOSE , undefined]
        , [SAX_VALUE_CLOSE , undefined]
        , [SAX_VALUE_CLOSE , undefined]
        , [SAX_VALUE_CLOSE , undefined]
        , [SAX_VALUE_CLOSE , undefined]


        ]
      }
    };

describe('clarinet', function() {

   var expectedEventNames = [ SAX_KEY
                            , SAX_VALUE_OPEN
                            , SAX_VALUE_CLOSE
                            , FAIL_EVENT
                            ];

  var times = false;
   for (var key in docs) {
     if(times) {
       continue;
     }
     times = true;

      var doc = docs[key];

      describe('case ' + key, function(){

         var bus = pubSub(),
             blackBoxRecording = eventBlackBox(bus, expectedEventNames);

         clarinet(bus);

         bus(STREAM_DATA).emit(doc.text);
         bus(STREAM_END).emit();

         it('should have the correct event types', function(){
            expect( blackBoxRecording ).toMatchOrder( doc.events );
         });

         doc.events.forEach(function( expectedEvent, i ){

            var blackBoxSlice = blackBoxRecording[i];

            // don't worry about the value for error events:
            if(blackBoxSlice.type != FAIL_EVENT ){
               it( i + 'th event should have the correct event value', function(){
                  expect( blackBoxSlice.val  ).toEqual( expectedEvent[1] );
               });
            }
         });

      });
   }

   beforeEach(function(){

     jasmine.addMatchers({
         toMatchOrder: function(){
           return {
             compare: function(actual, expected) {
                var result = {};

                var actualEventOrder = actual.map( function(e){
                   return e.type;
                });
                var expectedEventOrder = expected.map( function(a){
                   return a[0];
                });

                var lengthsMatch = actualEventOrder.length == expectedEventOrder.length;
                var everyEventMatches = actualEventOrder.every(function( actualEvent, i ){
                  return actualEvent == expectedEventOrder[i];
                });

                result.pass = lengthsMatch && everyEventMatches;
                if(!result.pass) {
                   result.message = 'events not in correct order. We have:\n' +
                     JSON.stringify(
                       actualEventOrder.map(prettyPrintEvent)
                     ) + '\nbut wanted:\n' +
                     JSON.stringify(
                       expectedEventOrder.map(prettyPrintEvent)
                     );
               }

               return result;

             }
           };
         }
     });
   });

});
