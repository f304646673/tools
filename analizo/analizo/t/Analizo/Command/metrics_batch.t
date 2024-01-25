package t::Analizo::Command::metrics_batch;
use strict;
use warnings;
use parent qw(Test::Analizo::Class);
use Test::More;
use Test::Analizo;
use Analizo;

BEGIN {
  use_ok 'Analizo::Command::metrics_batch'
};

sub constructor : Tests {
  my $analizo = Analizo->new;
  my ($cmd) = $analizo->prepare_command('metrics-batch');
  isa_ok($cmd, 'Analizo::Command::metrics_batch');
}

sub is_a_subclass_of_Analizo_Command : Tests {
  my $analizo = Analizo->new;
  my ($cmd) = $analizo->prepare_command('metrics-batch');
  isa_ok($cmd, 'Analizo::Command');
}

__PACKAGE__->runtests;
