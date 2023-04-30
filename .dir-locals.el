;;; Directory Local Variables
;;; For more information see (info "(emacs) Directory Variables")

((nil . (
	 (eval . (let
		     ((root
		       (projectile-project-root)))
		   (setq-local flycheck-clang-args
			       (list
				(concat "-I../")
				(concat "-std=c++20")))
		   (setq-local flycheck-clang-include-path
			       (list
				(concat "../")
				))
		   (setq-local flycheck-gcc-args
			       (list
				(concat "-I../")
				(concat "-std=c++20")
				))
		   (setq-local flycheck-gcc-include-path
			       (list
				(concat "../")
				))
		   ))))
 (c++-mode . ((c-basic-offset . 4)))
 (c-mode . ((mode . c++))))
