## Short Description

This repo contains code of a illustrative text ``compression/decompression'' utility oct, powered by a combination of arithmetic coding and character-level recurrent neural networks (RNN). Despite its simplicity, it beats gzip/bzip2 on compressing ratio for text files when RNN is trained against the compressed file. In other words, the use case currently supported is not a general compression, but rather a fixed-dataset compression; mainly due to a considerable time required for training a neural net. Such a scenario can be useful for storing Wikipedia on a mobile, for instance.

## Motivation

Arithmetic coding is a flexible compression approach that wraps any generative model P(next block | history) to build a compression mechanism. Better the model, better the compression level. One can use simple frequency models that consider the probability of observing a particular B p(B | history) to be equal to the frequency of B in history. While sometimes that might be good enough,  such an approach totally ignores sequential nature of data stream. In particular, that hold in textual data: a capital letter is likely to be observed after a dot and whitespace; 'e' is likely after 'th', and so on. Hopefully, you're convinced!

Recurrent neural nets (''vanilla'' RNN, GRU, and LSTM) proved to be very successful in text modelling. Character-level generative models pre-trained on various sources turned out to be capable of doing amazing stuff () and conquered the web in no time.

It looks like this is a match made in heaven, so I decided to play with character-level nets and arithmetic coding to see if something interesting can emerge. I started this mini-pet-project in May, and since then lot of stuff happened (I moved to France, finished and defended my thesis, Google wrote a paper on neural nets and arithmetic coding for image compression, etc). So I decided I'd rather dump it to Github as is, rather than hide it forever :)

## Broad overview of Machine Learning and Compression

### Five-line intro into Arithmetic Coding

### Five-line intro into RNNs

## How Code Is Organised

The code uses three external libs, plugged as submodules: picojson (C++ lib to parse json), rnn (re-implementation of char-rnn in keras; used only if you want to train your own models), and eigen (to implement forward-pass RNN and GRU). 

Further, there are two big, independent parts: the arithmetic coder/decoder (src/compression) and implementations of different models (src/models). The models implemented are: (a) constant model for debug purposes, (b) adaptive frequency model, (c) multi-layer vanilla RNN, and (d) multi-layer GRU.

## How can I launch it?
First, checkout the code:

```
git clone git@bitbucket.org:n0mad/octopi-research.git
cd octopi-research
git submodules init
git submodules update
```
and actually build it:
```
make
```

If everything is OK, we're ready to compress something. Let's do it:
```
./bin/octopi -c adaptive README.md README.md.oct
```
Here we compress the README.md file into README.md.oct using 'adaptive' model, which stands for a simple frequency model ( = 0-order Markov model). Now we can check the size of the original and resulting files:
```
ls -lh README.md README.md.oct
```
There should be some difference; on an early version of README.md I obtained 40% compression. Will it be the same after we decompress it? We can compare md5sum-s:
On Mac:
```
./bin/octopi -d adaptive README.md.oct README.md.new
md5 README.md.new README.md
```
On nix:
```
./bin/octopi -d adaptive README.md.oct README.md.new
md5sum README.md.new README.md
```
Yay, they're the same (at least they should be!)

Now we can proceed to a more fancy stuff. 

Download a part of pre-processed Simple Wikipedia here and a pre-trained 2-layer GRU model of layer size 128. Download an example here and the model here.
The file is compressed using the provided model. Note it's compressed size is 268K. Decompress it and check the size of the resulting file:
```
./bin/octopi -d ./gru128_2layer.json example.c example.d
ls -l example.d

```
I get something like 1087856 bytes. Given the compressed size of 274605, that corresponds to 25% compression ratio.

Let's compare to gzip
```
gzip -k -9 example.d && ls -l example.d.gz
```
The result is 398553, 37% compression ratio.

But gzip is a weak baseline, let's compare to bzip2:
```
bzip2 -k -9 ./example.d && ls -l example.d.bz2
```
In this case, we get some 308843 bytes, way better than gzip; 28% compression ratio.


But hey! we are *cheating*: we don't count the model storage cost to the compression ratio! Agree. A non-pruned two-layer GRU model with 128 units per layer, 204 input and output units (size of the alphabet used by our Wikipedia dump) would take 204x128 [input 'embedding' layer] + (128 * 128 + 128) * 4 [two GRU layers] + 128 * 204 + 204 [softmax layer], 4 bytes per number = roughly 460Kb (my current proof-of-concept stored these floats in json, which is not as compact it could be; however, it allows me to not-overcomplicate the POC.)
Huh, these 460Kb are totally not cost-effective for a 1Mb example! 

However, for a 100Mb full Simple Wikipedia (grab it here), that accounts for less than 0.5% of the original size, and hence the difference with bzip2 becomes significant.  On my system, the compressed size of Wikipedia is XXXX , including the 460Kb model it becomes ; giving the total compression ratio of ... In contrast, bzip2 compresses the same text into ...


## I want to train my own models and use them

## Conclusions

My personal take-away is that one can easily beat sophisticated general purpose compression algorithms without any knowledge of compression just by using some of-the-shelf techniques. Also RNNs are cool.


## Contributors
Eugene Kharitonov - firstname.lastname gmail
Tania Starikovskaia


## Related Work
