# How to Write a Spelling Corrector

[Peter Norvig](http://norvig.com/), Google’s Director of Research, wrote an article explaining how to write a spelling corrector. He wrote using python in 21 lines. After this, many people implemented in other languages, I wrote in C to compare the amount of lines and speed.

I quote some of Norvig’s paragraphs below:

“In the past week, two friends (Dean and Bill) independently told me they were amazed at how Google does spelling correction so well and quickly. Type in a search like [speling] and Google comes back in 0.1 seconds or so with Did you mean: spelling. (Yahoo and Microsoft are similar.) What surprised me is that I thought Dean and Bill, being highly accomplished engineers and mathematicians, would have good intuitions about statistical language processing problems such as spelling correction. But they didn’t, and come to think of it, there’s no reason they should: it was my expectations that were faulty, not their knowledge.

I figured they and many others could benefit from an explanation. The full details of an industrial-strength spell corrector like Google’s would be more confusing than enlightening, but I figured that on the plane flight home, in less than a page of code, I could write a toy spelling corrector that achieves 80 or 90% accuracy at a processing speed of at least 10 words per second.

You can find more details in [my original post](http://marcelotoledo.com/how-to-write-a-spelling-corrector/) or [Peter Norvig's Original Post](http://norvig.com/spell-correct.html).
