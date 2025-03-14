;;; Directory Local Variables            -*- no-byte-compile: t -*-
;;; For more information see (info "(emacs) Directory Variables")

((c++-mode . ((eval . (progn
                        (make-local-variable 'eglot-ignored-server-capabilities)
                        (add-to-list 'eglot-ignored-server-capabilities
                                     :documentOnTypeFormattingProvider)))
              (c-file-style . "filament")
              (apheleia-inhibit . t)))
 (c-mode . ((eval . (progn
                      (make-local-variable 'eglot-ignored-server-capabilities)
                      (add-to-list 'eglot-ignored-server-capabilities
                                   :documentOnTypeFormattingProvider)))
            (c-file-style . "filament")
            (apheleia-inhibit . t))))
