package t::Analizo::Command::metrics;
use strict;
use warnings;
use parent qw(Test::Analizo::Class);
use Test::More;
use Test::Analizo;
use Analizo;

BEGIN {
  use_ok 'Analizo::Command::metrics'
};

sub constructor : Tests {
  my $analizo = Analizo->new;
  my ($cmd) = $analizo->prepare_command('metrics');
  isa_ok($cmd, 'Analizo::Command::metrics');
}

sub is_a_subclass_of_Analizo_Command : Tests {
  my $analizo = Analizo->new;
  my ($cmd) = $analizo->prepare_command('metrics');
  isa_ok($cmd, 'Analizo::Command');
}

__PACKAGE__->runtests;
