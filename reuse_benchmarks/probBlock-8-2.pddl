(define (problem BLOCKS-8-2)
(:domain BLOCKS)
(:objects A B C D E F G H)
(:INIT (ON A H) (ON B E) (ONTABLE C) (ON D A) (ON E C) (ON F B) (ONTABLE G) (ON H G) (CLEAR D) (CLEAR F) (HANDEMPTY))
(:goal (AND (ON B A) (ON C B) (ON D C) (ON E D) (ON F E) (ON G F) (ON H G) ))
)