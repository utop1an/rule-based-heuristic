(define (problem BLOCKS-8-10)
(:domain BLOCKS)
(:objects A B C D E F G H)
(:INIT (ON A C) (ON B D) (ON C B) (ONTABLE D) (ON E H) (ON F A) (ON G E) (ON H F) (CLEAR G) (HANDEMPTY))
(:goal (AND (ON B A) (ON C B) (ON D C) (ON E D) (ON F E) (ON G F) (ON H G) ))
)