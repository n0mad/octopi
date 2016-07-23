import json
import numpy as np
import tensorflow as tf
import argparse
from six.moves import cPickle
import sys, os
sys.path += ['./rnn']
from model import Model
import re
from collections import defaultdict


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--save_dir', type=str, default='./data', help='model directory')
    parser.add_argument('--model_name', type=str, default='model.json', help='the output model filename')
    parser.add_argument('--model_type', type=str, default='rnn', help='type of the serialized model, (rnn|gru)')
    args = parser.parse_args()
    dump(args)

def dump(args):
    with open(os.path.join(args.save_dir, 'config.pkl'), 'rb') as f:
        saved_args = cPickle.load(f)
    with open(os.path.join(args.save_dir, 'chars_vocab.pkl'), 'rb') as f:
        chars, vocab = cPickle.load(f)
    model = Model(saved_args, True)


    rnnPattern = re.compile(r'rnnlm/MultiRNNCell/Cell(\d?)/BasicRNNCell/Linear/(Matrix|Bias):0')
    gruPattern = re.compile(r'rnnlm/MultiRNNCell/Cell(\d?)/GRUCell/(Gates|Candidate)/Linear/(Matrix|Bias):0')

    with tf.Session() as sess:
        tf.initialize_all_variables().run()
        saver = tf.train.Saver(tf.all_variables())
        ckpt = tf.train.get_checkpoint_state(args.save_dir)

        if ckpt and ckpt.model_checkpoint_path:
            #saver.restore(sess, os.path.join(args.save_dir, ckpt.model_checkpoint_path))
            saver.restore(sess, ckpt.model_checkpoint_path)
            data = {'rnnlm/embedding:0' : None, 'rnnlm/softmax_w:0' : None, 'rnnlm/softmax_b:0' : None}
            if args.model_type == 'rnn':
                layers = defaultdict(dict)
            if args.model_type == 'gru':
                layers = defaultdict(lambda: defaultdict(dict))

            for var in tf.all_variables():
                print var.name
                if var.name in data:
                    data[var.name] = var.eval().tolist()
                    continue

                if args.model_type == 'rnn':
                    m = rnnPattern.search(var.name)
                    if m:
                        layer = int(m.group(1))
                        content_type = m.group(2)
                        layers[layer][content_type] = var.eval().tolist()
                        continue

                if args.model_type == 'gru':
                    m = gruPattern.search(var.name)
                    if m:
                        layer = int(m.group(1))
                        gate = m.group(2)
                        matrix_or_bias = m.group(3)
                        layers[layer][gate][matrix_or_bias] = var.eval().tolist()
                        continue

            embedding = data['rnnlm/embedding:0']
            softmax_w = data['rnnlm/softmax_w:0']
            softmax_b = data['rnnlm/softmax_b:0']

            layer_size = len(embedding[0])
            dump = []
            dump += [layer_size]
            dump += [map(ord, chars)]
            dump += [embedding, softmax_w, softmax_b]

            if args.model_type == 'rnn':
                for layer_no in sorted(layers.keys()):
                    layer_data = layers[layer_no]['Matrix'], layers[layer_no]['Bias']
                    dump.append(layer_data)
            elif args.model_type == 'gru':
                for layer_no in sorted(layers.keys()):
                    layer_data = layers[layer_no]['Gates']['Matrix'], layers[layer_no]['Gates']['Bias'], layers[layer_no]['Candidate']['Matrix'], layers[layer_no]['Candidate']['Bias']
                    dump.append(layer_data)


            with open(args.model_name, 'w') as fout:
                fout.write(json.dumps(dump))

if __name__ == '__main__':
    main()
