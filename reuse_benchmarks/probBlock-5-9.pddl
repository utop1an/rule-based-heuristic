(define (problem BLOCKS-5-9)
(:domain BLOCKS)
(:objects A B C D E)
(:INIT (ON A E) (ONTABLE B) (ON C A) (ONTABLE D) (ONTABLE E) (CLEAR B) (CLEAR C) (CLEAR D) (HANDEMPTY))
(:goal (AND (ON A E) (ON B A) (ONTABLE C) (ON D B) (ONTABLE E) ))
)