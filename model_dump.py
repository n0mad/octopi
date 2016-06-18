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
    args = parser.parse_args()
    dump(args)

def dump(args):
    with open(os.path.join(args.save_dir, 'config.pkl'), 'rb') as f:
        saved_args = cPickle.load(f)
    with open(os.path.join(args.save_dir, 'chars_vocab.pkl'), 'rb') as f:
        chars, vocab = cPickle.load(f)
    model = Model(saved_args, True)


    rnnPattern = re.compile(r'rnnlm/MultiRNNCell/Cell(\d?)/BasicRNNCell/Linear/(Matrix|Bias):0')

    with tf.Session() as sess:
        tf.initialize_all_variables().run()
        saver = tf.train.Saver(tf.all_variables())
        ckpt = tf.train.get_checkpoint_state(args.save_dir)

        if ckpt and ckpt.model_checkpoint_path:
            saver.restore(sess, ckpt.model_checkpoint_path)
            data = {'rnnlm/embedding:0' : None, 'rnnlm/softmax_w:0' : None, 'rnnlm/softmax_b:0' : None}
            layers = defaultdict(dict)

            for var in tf.all_variables():
                if var.name in data:
                    data[var.name] = var.eval().tolist()
                    continue

                m = rnnPattern.search(var.name)
                if m:
                    layer = int(m.group(1))
                    content_type = m.group(2)
                    layers[layer][content_type] = var.eval().tolist()

            embedding = data['rnnlm/embedding:0']
            softmax_w = data['rnnlm/softmax_w:0']
            softmax_b = data['rnnlm/softmax_b:0']

            dump = []
            dump += [('chars', map(ord, chars))]
            dump += data.items()

            for layer_no in sorted(layers.keys()):
                layer_data = layers[layer_no].items()
                dump.append(layer_data)

            with open('model.json', 'w') as fout:
                fout.write(json.dumps(dump))

if __name__ == '__main__':
    main()
