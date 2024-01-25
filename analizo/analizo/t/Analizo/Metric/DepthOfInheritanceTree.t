package t::Analizo::Metric::DepthOfInheritanceTree;
use strict;
use warnings;
use parent qw(Test::Analizo::Class);
use Test::More;
use File::Basename;

use Analizo::Model;
use Analizo::Metric::DepthOfInheritanceTree;

eval('$Analizo::Metric::QUIET = 1;'); # the eval is to avoid Test::* complaining about possible typo

use vars qw($model $dit);

sub setup : Test(setup) {
  $model = Analizo::Model->new;
  $dit = Analizo::Metric::DepthOfInheritanceTree->new(model => $model);
}

sub use_package : Tests {
  use_ok('Analizo::Metric::DepthOfInheritanceTree');
}

sub has_model : Tests {
  is($dit->model, $model);
}

sub description : Tests {
  return "Depth of Inheritance Tree";
}

sub calculate : Tests {
  $model->add_inheritance('Level1', 'Level2');
  $model->add_inheritance('Level2', 'Level3');
  is($dit->calculate('Level1'), 2, 'DIT = 2');
  is($dit->calculate('Level2'), 1, 'DIT = 1');
  is($dit->calculate('Level3'), 0, 'DIT = 0');
}

sub calculate_with_multiple_inheritance : Tests {
  $model->add_inheritance('Level1', 'Level2A');
  $model->add_inheritance('Level1', 'Level2B');
  $model->add_inheritance('Level2B', 'Level3B');
  is($dit->calculate('Level1'), 2, 'with multiple inheritance take the larger DIT between the parents');
}


__PACKAGE__->runtests;

