(define (problem BLOCKS-4-5)
(:domain BLOCKS)
(:objects A B C D)
(:INIT (ONTABLE A) (ON B D) (ONTABLE C) (ON D A) (CLEAR B) (CLEAR C) (HANDEMPTY))
(:goal (AND (ON B A) (ON C B) (ON D C) ))
)