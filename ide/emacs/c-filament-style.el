;;; c-filament-style.el --- Filament C++ style -*- lexical-binding: t -*-

;; Copyright (C) 2023  Google LLC

;; Author: Eliza Velasquez
;; Version: 0.1.0
;; Created: 2023-10-31
;; Package-Requires: ((emacs "24.1"))
;; Keywords: c
;; URL: https://github.com/google/filament
;; SPDX-License-Identifier: Apache-2

;;; Commentary:

;; Defines a basic Filament style for C++ code in Emacs.

;;; Code:

(require 'cc-mode)
(defvar c-syntactic-context)

(defun c-filament-style-lineup-brace-list-intro (langelem)
  "Indent first line of braced lists in the Filament style.

This properly indents doubled-up arglists + lists, e.g. ({.  It
also properly indents enums at 1x and other lists at 2x.

LANGELEM is the cons of the syntactic symbol and the anchor
position (or nil if there is none)."
  (save-excursion
    (let (case-fold-search)
      (goto-char (c-langelem-pos langelem))
      (if (looking-at "enum\\b")
          c-basic-offset
        (if (assq 'arglist-cont-nonempty c-syntactic-context)
            (- c-basic-offset)
          (* 2 c-basic-offset))))))

(defun c-filament-style-lineup-brace-list-entry (_langelem)
  "Indent following lines in braced lists in the Filament style.

This properly indents doubled-up arglists + lists, e.g. ({."
  (if (assq 'arglist-cont-nonempty c-syntactic-context)
      (- (* c-basic-offset 2))
    0))

(defun c-filament-style-lineup-arglist (langelem)
  "Indent following lines in braced lists in the Filament style.

This properly indents arglists nested in if statements.  LANGELEM
is the cons of the syntactic symbol and the anchor position (or
nil if there is none)."
  (save-excursion
    (let (case-fold-search)
      (goto-char (c-langelem-pos langelem))
      (if (and (cdr c-syntactic-context)
               (looking-at "if\\b"))
          c-basic-offset
        (* 2 c-basic-offset)))))

(c-add-style "filament"
             '((c-basic-offset . 4)
               (c-offsets-alist
                (innamespace . 0)
                (inextern-lang . 0)
                (arglist-intro . c-filament-style-lineup-arglist)
                (arglist-cont . 0)
                (arglist-cont-nonempty . c-filament-style-lineup-arglist)
                (arglist-close . c-filament-style-lineup-arglist)
                (statement-cont . ++)
                (case-label . +)
                (brace-list-intro . c-filament-style-lineup-brace-list-intro)
                (brace-list-entry . c-filament-style-lineup-brace-list-entry)
                (brace-list-close . c-filament-style-lineup-brace-list-entry)
                (label . [0])
                (member-init-intro . ++))))

(provide 'c-filament-style)

;;; c-filament-style.el ends here
