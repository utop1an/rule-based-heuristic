(define (problem BLOCKS-4-10)
(:domain BLOCKS)
(:objects A B C D)
(:INIT (ON A D) (ONTABLE B) (ON C B) (ON D C) (CLEAR A) (HANDEMPTY))
(:goal (AND (ON B A) (ON C B) (ON D C) ))
)