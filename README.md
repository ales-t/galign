GAlign – word alignment using HMMs estimated with Gibbs sampling
================================================================

Description
-----------

GAlign is yet another word alignment tool (like GIZA++, MGiza, fast_align,...)
with several distinguishing features:

* competitive alignment quality
* supports incremental training and forced alignment
* fast performance thanks to multi-threading in all stages, scales to many cores
* easy to use
* simple code base with minimal dependencies (Boost, Intel TBB)

Installation
------------

Download and compile GAlign:

```
git clone https://github.com/ales-t/galign.git
cd galign
# export BOOST_PATH=/optionally/specify/different/path/to/Boost
make
```

Verify that you can run the program:

```
bin/wordalign --help
```

Usage
-----

Use of GAlign can be roughly divided into 3 modes:

### Train and align (+output model)

Basic usage:

```
paste corpus.src corpus.tgt | bin/wordalign -m > corpus.alignment
```

For greater speed, you might like to skip the final Viterbi search for the best
alignment (and use the last Gibbs sample instead) with the switch ```--no-viterbi```.

Train and store the model:

```
paste corpus.src corpus.tgt | bin/wordalign -m --store-model-file corpus.model > corpus.alignment
```

### Continue training with an existing model

In this mode, the existing model will be loaded and used as a starting point for training. Note that data
size matters: if the old data are very small compared to the new corpus, statistics collected from the
new data will outweigh the existing model, and vice versa.

```
paste newcorpus.src newcorpus.tgt | bin/wordalign -m --load-model-file oldcorpus.model > newcorpus.alignment
```

### Force alignment

Do not train, only search for the best alignment given the existing model. Avoid ```--no-viterbi``` in this mode.

```
paste newcorpus.src newcorpus.tgt | bin/wordalign -f -m --load-model-file oldcorpus.model > newcorpus.alignment
```

License
-------

Author: Aleš Tamchyna <tamchyna -at- ufal.mff.cuni.cz>

Copyright © 2013 Institute of Formal and Applied Linguistics,
   Charles University in Prague.

Licensed under the GNU Lesser General Public Licence version 2.1.
