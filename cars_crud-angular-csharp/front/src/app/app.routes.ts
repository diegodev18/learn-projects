import { Routes } from '@angular/router';

export const routes: Routes = [
  {
    path: '',
    loadChildren: () =>
      import('./features/home/home-routing.module').then(
        (m) => m.HomeRoutingModule
      ),
  },
];
